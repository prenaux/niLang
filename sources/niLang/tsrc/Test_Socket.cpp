#include "stdafx.h"
#include "../src/API/niLang/ISocket.h"

#if !defined niNoSocket

#include "../src/API/niLang/Utils/RemoteAddress.h"
#include "../src/API/niLang/Utils/Random.h"
#include "../src/API/niLang/Utils/SyncPtr.h"
#include "../src/API/niLang/Utils/TimerSleep.h"
#include "../src/API/niLang/Utils/ConcurrentImpl.h"
#include "../src/API/niLang/IFile.h"
#include "../src/API/niLang/STL/scope_guard.h"

const ni::achar* _kaszTestString = _A("Test string");
const ni::achar* _kaszTestString2 = _A("Second Test string");
const ni::achar* _kaszTestString3 = _A("Third Test string");

static struct _SocketGetRandomPort {
  _SocketGetRandomPort() {
  }
  ni::tU16 GetPort() const {
    ni::tU64 seed = ni::ni_prng_get_seed_from_maybe_secure_source();
    niDebugFmt(("SEED: %d\n",seed));
    ni::RandSeed(seed);
    ni::tU16 p = (ni::tU16)ni::RandIntRange(40000,44000);
    niDebugFmt(("SOCKET USING PORT: %d\n",p));
    return p;
  }
} _socketRandomizePort;

static ni::tU16 _GetTestSocketPort() {
  // We randomize the port number because some OS, such as Linux,
  // lock the socket for a while before allowing it to be reused.
  static ni::tU16 _port = _socketRandomizePort.GetPort();
  return _port;
}

//--------------------------------------------------------------------------------------------
//
//  UDP unreliable socket test
//
//--------------------------------------------------------------------------------------------

struct FSocket_UDP {};

///////////////////////////////////////////////
TEST_FIXTURE(FSocket_UDP,QueryInterface) {
  ni::Ptr<ni::iSocket> socket = ni::GetLang()->CreateSocket(ni::eSocketProtocol_UDP,NULL);
  CHECK(socket.IsOK());
  CHECK_EQUAL((ni::tU32)ni::eSocketProtocol_UDP,(ni::tU32)socket->GetProtocol());
  CHECK(!!ni::QueryInterface<ni::iUnknown>(socket));
  CHECK(!!ni::QueryInterface<ni::iSocket>(socket));
}

///////////////////////////////////////////////
TEST_FIXTURE(FSocket_UDP,Bind) {
  ni::Ptr<ni::iRemoteAddressIPv4> ra = ni::GetLang()->CreateRemoteAddressIPv4(0,0);
  ra->SetHost(niFourCC(127,0,0,1));
  ra->SetPort(_GetTestSocketPort());

  ni::Ptr<ni::iSocket> recv = ni::GetLang()->CreateSocket(ni::eSocketProtocol_UDP,NULL);
  CHECK(recv.IsOK());
  CHECK(recv->Bind(ra) == ni::eTrue);
}

///////////////////////////////////////////////
TEST_FIXTURE(FSocket_UDP,BindPortAny) {
  ni::Ptr<ni::iSocket> recv = ni::GetLang()->CreateSocket(ni::eSocketProtocol_UDP,NULL);
  CHECK(recv.IsOK());
  CHECK(recv->BindPortAny(_GetTestSocketPort()) == ni::eTrue);
}

///////////////////////////////////////////////
TEST_FIXTURE(FSocket_UDP,LoopbackSendRecv) {
  ni::Ptr<ni::iRemoteAddressIPv4> ra = ni::GetLang()->CreateRemoteAddressIPv4(0,0);
  ra->SetHost(niFourCC(127,0,0,1));
  ra->SetPort(_GetTestSocketPort());

  // create the receiver
  ni::Ptr<ni::iSocket> recv = ni::GetLang()->CreateSocket(ni::eSocketProtocol_UDP,NULL);
  CHECK(recv.IsOK());
  CHECK(recv->Bind(ra) == ni::eTrue);

  // create the sender
  ni::Ptr<ni::iSocket> send = ni::GetLang()->CreateSocket(ni::eSocketProtocol_UDP,NULL);
  CHECK(send.IsOK());
  CHECK(send->Connect(ra) == ni::eTrue);

  // send data
  {
    ni::Ptr<ni::iFile> sendData = ni::CreateFileDynamicMemory(256,NULL);
    sendData->SetTextEncodingFormat(ni::eTextEncodingFormat_UTF8);
    sendData->WriteString(_kaszTestString);
    sendData->SeekSet(0);
    // We can't use SendTo here, even with the same connected IP. It fails
    // with a eSocketErrno_AlreadyConnected on OSX, which is consistent
    // with the specs of the BSD socket...
    CHECK_EQUAL(sendData->GetSize(),send->Send(sendData,0));
    CHECK_EQUAL((ni::tU32)ni::eSocketErrno_OK,(ni::tU32)send->GetErrno());
  }

  // wait, make sure the sending socket has some time to actually do the sending
  ni::SleepMs(10);

  // receive data
  {
    ni::Ptr<ni::iRemoteAddressIPv4> raSender = ni::GetLang()->CreateRemoteAddressIPv4(0,0);
    ni::Ptr<ni::iFile> recvData = ni::CreateFileDynamicMemory(256,NULL);
    CHECK(recv->Wait(ni::eSocketWaitFlags_Receive) == ni::eSocketWaitFlags_Receive);
    if (recv->Wait(ni::eSocketWaitFlags_Receive) == ni::eSocketWaitFlags_Receive)
    {
      const ni::tI32 res = recv->ReceiveFrom(recvData,raSender);
      CHECK_EQUAL((ni::tU32)ni::eSocketErrno_OK,(ni::tU32)recv->GetErrno());
      CHECK_EQUAL(ni::StrLen(_kaszTestString),res);
      recvData->SeekSet(0);

      ni::cString strData = recvData->ReadString();
      niDebugFmt(("... strData: %s", strData));
      CHECK_EQUAL(_ASTR(_kaszTestString),_ASTR(strData));
    }
  }
}

//--------------------------------------------------------------------------------------------
//
//  TCP reliable socket test
//
//--------------------------------------------------------------------------------------------

struct FSocket_TCP {};

///////////////////////////////////////////////
TEST_FIXTURE(FSocket_TCP,QueryInterface) {
  ni::Ptr<ni::iSocket> socket = ni::GetLang()->CreateSocket(ni::eSocketProtocol_TCP,NULL);
  CHECK(socket.IsOK());
  CHECK_EQUAL((ni::tU32)ni::eSocketProtocol_TCP,(ni::tU32)socket->GetProtocol());
  CHECK(!!ni::QueryInterface<ni::iUnknown>(socket));
  CHECK(!!ni::QueryInterface<ni::iSocket>(socket));
}

///////////////////////////////////////////////
struct SimpleTCPServer : public ni::cIUnknownImpl<ni::iRunnable,ni::eIUnknownImplFlags_Default> {
  SimpleTCPServer() : _error(_H("")) {}

  ni::Var __stdcall Run() override {
    niDefer {
      _eventListening.Signal();
    };

    ni::Ptr<ni::iSocket> socketConnect = ni::GetLang()->CreateSocket(ni::eSocketProtocol_TCP,NULL);
    // bind to the connection port
    if (!socketConnect->BindPortAny(_GetTestSocketPort())) {
      _error = _H("Can't bind port");
      niError(niHStr(_error));
      return 0;
    }

    // listen for connection
    if (!socketConnect->Listen(0)) {
      _error = _H("Can't listen connection");
      niError(niHStr(_error));
      return 0;
    }

    _eventListening.Signal();

    // accept connection
    niDebugFmt((_A("SERVER: Accept connection...")));
    ni::Ptr<ni::iSocket>  socketClient;
    const ni::tF64 timerAccept = ni::TimerInSeconds();
    while (1) {
      if (socketConnect->Wait(ni::eSocketWaitFlags_Receive) == ni::eSocketWaitFlags_Receive) {
        socketClient = socketConnect->Accept();
        break;
      }
      if ((ni::TimerInSeconds()-timerAccept) > 0.5f) {
        break;
      }
    }
    if (!socketClient.IsOK()) {
      _error = _H("Can't accept connection");
      return 0;
    }

    // receive the data
    niDebugFmt((_A("SERVER: Waiting for client data...")));
    ni::Ptr<ni::iFile> file = ni::CreateFileDynamicMemory(0,NULL);
    if (socketClient->Receive(file) <= 0) {
      _error = _H(niFmt(_A("Can't receive data, errno: %d"),socketClient->GetErrno()));
      return 0;
    }
    file->SeekSet(0);
    _received = file->ReadString();

    // send back data
    niDebugFmt((_A("SERVER: Sending data to client...")));
    file->SeekSet(0);
    file->WriteStringZ(_kaszTestString2);
    file->SeekSet(0);
    if (socketClient->Send(file,0) <= 0) {
      _error = _H(niFmt(_A("Can't send data, errno: %d"),socketClient->GetErrno()));
      return 0;
    }

    // done
    niDebugFmt((_A("SERVER: Done...")));
    return 0;
  }

  ni::tHStringPtr _error;
  ni::ThreadEvent _eventListening;
  ni::cString     _received;
};

struct SimpleTCPClient : public ni::cIUnknownImpl<ni::iRunnable,ni::eIUnknownImplFlags_Default> {
  SimpleTCPClient() : _error(_H("")) {}
  ni::Var __stdcall Run() override {
    ni::Ptr<ni::iSocket>  socket = ni::GetLang()->CreateSocket(ni::eSocketProtocol_TCP,NULL);

    // connect to the server
    niDebugFmt((_A("CLIENT: Connecting...")));
    ni::Ptr<ni::iRemoteAddressIPv4> serverAddr = ni::GetLang()->CreateRemoteAddressIPv4(niFourCC(127,0,0,1),_GetTestSocketPort());
    if (!socket->Connect(serverAddr)) {
      _error = _H("Can't connect to server");
      return 0;
    }

    // send the data
    niDebugFmt((_A("CLIENT: Sending data...")));
    ni::Ptr<ni::iFile> file = ni::CreateFileDynamicMemory(0,NULL);
    file->WriteStringZ(_kaszTestString);
    file->SeekSet(0);
    if (socket->Send(file,0) <= 0) {
      _error = _H(niFmt(_A("Can't send data, errno: %d"),socket->GetErrno()));
      return 0;
    }

    // receive data
    niDebugFmt((_A("CLIENT: Receiving data...")));
    file->SeekSet(0);
    if (socket->Receive(file) <= 0) {
      _error = _H(niFmt(_A("Can't receive data, errno: %d"),socket->GetErrno()));
      return 0;
    }

    file->SeekSet(0);
    _received = file->ReadString();
    niDebugFmt((_A("CLIENT: Done...")));
    return 0;
  }

  ni::tHStringPtr _error;
  ni::cString     _received;
};

///////////////////////////////////////////////
TEST_FIXTURE(FSocket_TCP,Connect) {
  ni::Ptr<SimpleTCPServer> server = niNew SimpleTCPServer();
  ni::Ptr<SimpleTCPClient> client = niNew SimpleTCPClient();

  ni::Ptr<ni::iFuture> serverThreadFuture = ni::GetConcurrent()->ThreadRun(server);

  // wait until the server accepts connections
  niDebugFmt((_A("SERVER-THREAD: Waiting for server...")));
  CHECK(server->_eventListening.Wait(5000));
  niDebugFmt((_A("SERVER-THREAD: Done...")));

  if (ni::HStringIsEmpty(server->_error)) {
    ni::Ptr<ni::iFuture> clientThreadFuture = ni::GetConcurrent()->ThreadRun(client);
    // wait until the client is done
    niDebugFmt((_A("CLIENT-THREAD: Waiting for client...")));
    clientThreadFuture->Wait(ni::eInvalidHandle);
    niDebugFmt((_A("CLIENT-THREAD: Done...")));
  }

  ni::tHStringPtr hspEmpty = _H("");

  // wait until the server is done
  serverThreadFuture->Wait(ni::eInvalidHandle);
  CHECK_EQUAL(hspEmpty.ptr(),client->_error.ptr());
  CHECK_EQUAL(hspEmpty.ptr(),server->_error.ptr());
  CHECK_EQUAL(ni::cString(_kaszTestString),server->_received);
  CHECK_EQUAL(ni::cString(_kaszTestString2),client->_received);
}

/////////////////////////////////////////////////////
TEST_FIXTURE(FSocket_TCP,KeepAlive) {
  ni::Ptr<ni::iSocket> ptrSocket = ni::GetLang()->CreateSocket(ni::eSocketProtocol_TCP,NULL);
  CHECK(ptrSocket.IsOK());
  CHECK(ptrSocket->SetKeepAlive(ni::eTrue));
  CHECK(ptrSocket->GetIsKeepAlive());
  {
    AUTO_WARNING_MODE();
    CHECK(ptrSocket->SetKeepAliveParameters(1000,2000,6));
    CHECK_EQUAL(1000,ptrSocket->GetKeepAliveTime());
    CHECK_EQUAL(2000,ptrSocket->GetKeepAliveInterval());
    CHECK_EQUAL(6,ptrSocket->GetKeepAliveProbes());
  }
}

/////////////////////////////////////////////////////
TEST_FIXTURE(FSocket_TCP,ReuseAddress) {
  ni::Ptr<ni::iRemoteAddressIPv4> ra = ni::GetLang()->CreateRemoteAddressIPv4(0,44719);
  ni::Ptr<ni::iSocket> ptrSocket = ni::GetLang()->CreateSocket(ni::eSocketProtocol_TCP,ra);
  CHECK(ptrSocket.IsOK());
  CHECK(ptrSocket->SetReuseAddress(ni::eTrue));
  CHECK(ptrSocket->GetReuseAddress());
  CHECK(ptrSocket->SetReuseAddress(ni::eFalse));
  CHECK(!ptrSocket->GetReuseAddress());
}
#endif
