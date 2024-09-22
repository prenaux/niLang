#pragma once
#ifndef __TNIFILETRANSPORT_H_B35DE076_218F_FF45_816D_130C0CFB7C2A__
#define __TNIFILETRANSPORT_H_B35DE076_218F_FF45_816D_130C0CFB7C2A__

#include <niLang/Types.h>
#include <niLang/IFile.h>

#include <thrift/transport/TTransport.h>

namespace ni {

using namespace apache::thrift::transport;

struct TNiFileReaderTransport : public TTransport {
  TNiFileReaderTransport(ni::iFile* apFile)
      : _readBuffer(apFile) {
  }

  virtual ~TNiFileReaderTransport() {
  }

  void open() override {
  }

  bool isOpen() const override {
    return true;
  }

  bool peek() override {
    return true;
  }

  void close() override {
  }

  uint32_t read_virt(uint8_t* buf, uint32_t len) override {
    niPanicAssertMsg(_readBuffer.IsOK(), 0);
    // niDebugFmt(("... Read: len: %d", len));
    uint32_t nread = niUnsafeNarrowCast(uint32_t, _readBuffer->ReadRaw(buf, len));
    return nread;
  }
  uint32_t readAll_virt(uint8_t* buf, uint32_t len) override {
    niPanicAssertMsg(_readBuffer.IsOK(), 0);
    // niDebugFmt(("... ReadAll: len: %d", len));
    uint32_t nread = niUnsafeNarrowCast(uint32_t, _readBuffer->ReadRaw(buf, len));
    return nread;
  }

  uint32_t readEnd() override {
    _readBuffer->SeekSet(0);
    return 0;
  }

  void write_virt(const uint8_t*, uint32_t) override {
    niPanicUnreachable("Can't write in TNiFileReaderTransport.");
  }
  uint32_t writeEnd() override {
    return 0;
  }

  void flush() override {
  }

  virtual const std::string getOrigin() const override {
    return "TNiFileReaderTransport";
  }

  ni::Ptr<ni::iFile> _readBuffer;
};

struct TNiFileWriterTransport : public TTransport {
  TNiFileWriterTransport(ni::iFile* apFile)
      : _writeBuffer(apFile) {
  }

  virtual ~TNiFileWriterTransport() {
  }

  void open() override {
  }

  bool isOpen() const override {
    return true;
  }

  bool peek() override {
    return true;
  }

  void close() override {
  }

  uint32_t read_virt(uint8_t*, uint32_t) override {
    niPanicUnreachable("Can't read from TNiFileWriterTransport.");
    return 0;
  }
  uint32_t readAll_virt(uint8_t*, uint32_t) override {
    niPanicUnreachable("Can't readAll from TNiFileWriterTransport.");
    return 0;
  }

  uint32_t readEnd() override {
    return 0;
  }

  void write_virt(const uint8_t* buf, uint32_t len) override {
    // niDebugFmt(("... Write: len: %d, %c (%d)", len, buf[0], buf[0]));
    _writeBuffer->WriteRaw(buf, len);
  }
  uint32_t writeEnd() override {
    return 0;
  }

  void flush() override {
  }

  virtual const std::string getOrigin() const override {
    return "TNiFileWriterTransport";
  }

  ni::Ptr<ni::iFile> _writeBuffer;
};

}
#endif // __TNIFILETRANSPORT_H_B35DE076_218F_FF45_816D_130C0CFB7C2A__
