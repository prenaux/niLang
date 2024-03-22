#include "stdafx.h"
#include "../src/API/niLang/STL/scope_guard.h"
#include "../src/API/niLang/Utils/DataTableUtils.h"

using namespace ni;

static const cString _dtXML = R"""(<Entity classname="Dummy" name="ExtCamOrbitTarget"><Base group="Dummy" flags="" enabled="1"/><Transform local_matrix="Matrix(-0.674032,-2.03567e-009,0.738702,0,-0.0666186,0.995925,-0.0607864,0,-0.735692,-0.0901833,-0.671285,0,0,20,0,1)" scale="Vec3(1,1,1)" flags="InheritPositionX|InheritPositionY|InheritPositionZ|InheritRotation"/><Children><Entity classname="Camera" name="ExtOrbit"><Base group="camera" flags="Relative" enabled="1"/><Transform local_matrix="Matrix(0.887373,0.00934788,-0.460956,0,1.61839e-008,0.999794,0.0202751,0,0.461051,-0.0179916,0.887191,0,-122.582,4.78352,-235.882,1)" scale="Vec3(1,1,1)" flags="InheritPositionX|InheritPositionY|InheritPositionZ|InheritRotation"/><Camera fov="45" ortho_size="100" near_clip_plane="1" far_clip_plane="1e+006" frustum_culling="1" occlusion_culling="1" orthogonal="0"/><Timeline><Timelines><Timeline name="Default" play="0" stop_when_reset="0" loop="0" reversed="1" ping_pong_loop="0" speed="1" start_time="7" end_time="8" current_time="7.5" event_class=""/></Timelines></Timeline></Entity></Children><Timeline><Timelines><Timeline name="Default" play="0" stop_when_reset="0" loop="0" reversed="0" ping_pong_loop="0" speed="1" start_time="7" end_time="8" current_time="7.5" event_class=""/><Timeline name="FingerRelativeMove" play="0" stop_when_reset="1" loop="1" reversed="0" ping_pong_loop="0" speed="1" start_time="7" end_time="8" current_time="7" event_class="FingerRelativeMove" Finger="Touch0|MouseLeft|PointerLocked" WeightFiltering="0.2" Sensitivity="2" Acceleration="20" AccelerationBase="0.2" AccelerationMax="0.3" TriggerEveryTime="0" StopWhenReleased="1"><Action name="InputPitch" desc="InputPitch" start_time="7" end_time="8" enabled="1" target="" speed="1" view_range="Vec2(0,10313.2)"/><Action name="InputYaw" desc="InputYaw" start_time="7" end_time="8" enabled="1" target="" speed="1" view_range="Vec2(0,0)"/></Timeline></Timelines></Timeline></Entity>)""";

static const cString _dtXML_Base64Compressed = "hwcAAHjavZVrT9swFIb/iuVPoJVgJ81NaisxLhISaAiqMfYlMulpa82NI9spMPHjd5wGemEXTRr70tqvj31eP/ZxBqeVk+6JlEpYW4kFDOlJs1g8UbLqnD66Y7H4ZO6lGwszA0dHg4/CApkZ3dSvwVMlZnZIKYFK3CuYDCmnh6PB2IjKTrVZEKVLoYqFcEY+Dull+793wIIk7bMo7B2EAYviJIUDxvIeC9IoS1nYYz0MYUmS8CxBNc/jPIxXGkuzpL8KSHFmHrZyzngWRb12YR5mMQawXuh/+D4lFj3gnj5DGe3xHm+1zvl5NQcj3ZW20kldfXneEe52ha8vwrV2wgt+v8dzqSYGqtHgLVbECEZscG2h7uAsu6DO1TUoXHsJf8mVBVmWRmmEyBBn1E+zzCPpJyyPE88iQKBR7mFnLdY8zfs+OGRhGnMPDYM5w6aHytM8554/rspzP3zAwzCIs7DXD9Isiv35RTEOh/8Jc0uJTPVySPsxJdq4uS6s/I5ZOWPIGIQpSiXrolai8io62JXgA2MJ6qaxrlkUZaOUrGZtrC5L1VjMtq36NDNdCTWkrD0GuQAchXXLrpvdQZ/AVDTKUYJpn/w8Yp2ui4c5VIUBC67VlNZ12zCwBGNXB01qzFzUGn9ex20N3aB1wrjCSZ8k9Rdk0nUySsrG4C18HQ2QEa6LQnsfsU7R/OGG58P1Tg5XNxcb68v8rvtk77zPHZ9nmAvMS2Fdal9cv7bMXyzzd7K8Y/hn7lbakI51U87Z86VuLFzA1D1faVk5MBe6/AYTSm5BzubuTCrU2ivLgpCSG6h8JS3xTIcU+0dlCQqrx9cSCmxb8W9RN3FTvhSPXo0oGRs5QzeniOJp3G4BV7hBbLdIDX2D6C7vaHBU+qkd9vOqbtyVdOWckgnYclv5HbCNl4+49hPkvzNr2EsJDwW+h7PVkxPu4fvGIh4F4b4//rcu7sTDloe2/48dsP2tEvtjtf0A80hu6Q==";

static const cString _dtXML_Prop_with_space = R"""(<Properties Bytecollider="123" Byte Collider="456" ByteCollider = "789"/>)""";

struct FDTSerialize {
};

TEST_FIXTURE(FDTSerialize,Types) {
  Ptr<iDataTable> dt = ni::CreateDataTable("DT");
  tU32 i;

  i = dt->SetInt("0", niMinI32);
  CHECK_EQUAL(eDataTablePropertyType_Int32, dt->GetVarFromIndex(i).GetType());
  i = dt->SetInt("0", niMaxI32);
  CHECK_EQUAL(eDataTablePropertyType_Int32, dt->GetVarFromIndex(i).GetType());

  i = dt->SetInt("0", niMinI64);
  CHECK_EQUAL(eDataTablePropertyType_Int64, dt->GetVarFromIndex(i).GetType());
  i = dt->SetInt("0", niMaxI64);
  CHECK_EQUAL(eDataTablePropertyType_Int64, dt->GetVarFromIndex(i).GetType());

  i = dt->SetInt("0", (tI64)niMinI32-1);
  CHECK_EQUAL(eDataTablePropertyType_Int64, dt->GetVarFromIndex(i).GetType());
  CHECK_EQUAL((tI64)niMinI32-1, dt->GetIntFromIndex(i));

  i = dt->SetInt("0", (tI64)niMaxI32+1);
  CHECK_EQUAL(eDataTablePropertyType_Int64, dt->GetVarFromIndex(i).GetType());
  CHECK_EQUAL((tI64)niMaxI32+1, dt->GetIntFromIndex(i));

  i = dt->SetInt("0", 123);
  CHECK_EQUAL(eDataTablePropertyType_Int32, dt->GetVarFromIndex(i).GetType());
  i = dt->SetInt("0", -123);
  CHECK_EQUAL(eDataTablePropertyType_Int32, dt->GetVarFromIndex(i).GetType());
}

TEST_FIXTURE(FDTSerialize,Path) {
  const cString xml = R"(
<Root>
  <C1 S='123'>
    <GC1 F='1.23' />
  </C1>
  <C2 />
  <C2 U='123'>
    <GC2 B='1' />
    <GC3 V2='Vec2(1,1)' V3='Vec3(1,1,1)' />
    <GC3 />
  </C2>
</Root>
)";
  auto dtRoot = ni::CreateDataTableFromXML(xml.Chars());
  auto dtC1 = dtRoot->GetChild("C1");
  auto dtGC1 = dtC1->GetChild("GC1");
  auto dtC2 = dtRoot->GetChildFromIndex(2); // Target by index to avoid the 'C2' dummy
  auto dtGC2 = dtC2->GetChild("GC2");
  auto dtGC3 = dtC2->GetChild("GC3");

  const cString s = dtC1->GetString("S");
  const tF32 f = dtGC1->GetString("F").Float();
  const tU32 u = dtC2->GetString("U").ULong();
  const tBool b = dtGC2->GetString("B").Bool();
  const sVec2f v2 = dtGC3->GetString("V2").Vec2<tF32>();
  const sVec3f v3 = dtGC3->GetString("V3").Vec3<tF32>();

  const Var nullS("");
  CHECK(s.Eq(dtRoot->GetVarFromPath("C1/@S", nullS).GetString()));
  CHECK(s.Eq(dtC1->GetVarFromPath("/C1/@S", nullS).GetString()));
  CHECK(s.Eq(dtRoot->GetVarFromPath("[0]/@S", nullS).GetString()));
  CHECK(s.Eq(dtC1->GetVarFromPath("@S", nullS).GetString()));
  CHECK_EQUAL(u, dtRoot->GetVarFromPath("C2[1]/@U", nullS).GetString().ULong());
  CHECK_EQUAL(nullS, dtRoot->GetVarFromPath("C2/@U", nullS)); // This should get the first C2
  CHECK_EQUAL(nullS, dtRoot->GetVarFromPath("C2[0]/@U", nullS)); // This should get the first C2
  CHECK_EQUAL(u, dtRoot->GetVarFromPath("[2]/@U", nullS).GetString().ULong());
  CHECK_EQUAL(u, dtC2->GetVarFromPath("@U", nullS).GetString().ULong());
  CHECK_EQUAL(f, dtRoot->GetVarFromPath("C1/GC1/@F", nullS).GetString().Float());
  CHECK_EQUAL(f, dtRoot->GetVarFromPath("C1/[0]/@F", nullS).GetString().Float());
  CHECK_EQUAL(f, dtRoot->GetVarFromPath("[0]/GC1/@F", nullS).GetString().Float());
  CHECK_EQUAL(f, dtRoot->GetVarFromPath("[0]/[0]/@F", nullS).GetString().Float());
  CHECK_EQUAL(f, dtC1->GetVarFromPath("GC1/@F", nullS).GetString().Float());
  CHECK_EQUAL(f, dtC1->GetVarFromPath("[0]/@F", nullS).GetString().Float());
  CHECK_EQUAL(f, dtGC1->GetVarFromPath("@F", nullS).GetString().Float());
  CHECK_EQUAL(b, dtRoot->GetVarFromPath("C2[1]/GC2/@B", nullS).GetString().Bool());
  CHECK_EQUAL(b, dtRoot->GetVarFromPath("[2]/GC2/@B", nullS).GetString().Bool());
  CHECK_EQUAL(b, dtRoot->GetVarFromPath("[2]/[0]/@B", nullS).GetString().Bool());
  CHECK_EQUAL(b, dtC2->GetVarFromPath("[0]/@B", nullS).GetString().Bool());
  CHECK_EQUAL(b, dtGC2->GetVarFromPath("@B", nullS).GetString().Bool());
  CHECK_EQUAL(v2, dtRoot->GetVarFromPath("C2[1]/GC3/@V2", nullS).GetString().Vec2<tF32>());
  CHECK_EQUAL(v2, dtRoot->GetVarFromPath("[2]/GC3/@V2", nullS).GetString().Vec2<tF32>());
  CHECK_EQUAL(v2, dtRoot->GetVarFromPath("[2]/[1]/@V2", nullS).GetString().Vec2<tF32>());
  CHECK_EQUAL(v2, dtC2->GetVarFromPath("[1]/@V2", nullS).GetString().Vec2<tF32>());
  CHECK_EQUAL(v2, dtGC3->GetVarFromPath("@V2", nullS).GetString().Vec2<tF32>());
  CHECK_EQUAL(v3, dtRoot->GetVarFromPath("C2[1]/GC3/@V3", nullS).GetString().Vec3<tF32>());
  CHECK_EQUAL(v3, dtRoot->GetVarFromPath("[2]/GC3/@V3", nullS).GetString().Vec3<tF32>());
  CHECK_EQUAL(v3, dtRoot->GetVarFromPath("[2]/[1]/@V3", nullS).GetString().Vec3<tF32>());
  CHECK_EQUAL(v3, dtC2->GetVarFromPath("[1]/@V3", nullS).GetString().Vec3<tF32>());
  CHECK_EQUAL(v3, dtGC3->GetVarFromPath("@V3", nullS).GetString().Vec3<tF32>());

  cString pS,pU,pF,pB,pV2,pV3;
  pS = GetLang()->GetAbsoluteDataTablePath(dtC1, 0);
  pU = GetLang()->GetAbsoluteDataTablePath(dtC2, 0);
  pF = GetLang()->GetAbsoluteDataTablePath(dtGC1, 0);
  pB = GetLang()->GetAbsoluteDataTablePath(dtGC2, 0);
  pV2 = GetLang()->GetAbsoluteDataTablePath(dtGC3, 0);
  pV3 = GetLang()->GetAbsoluteDataTablePath(dtGC3, 1);
  CHECK(pS.Eq("/C1/@S"));
  CHECK(pF.Eq("/C1/GC1/@F"));
  CHECK(pU.Eq("/C2[1]/@U"));
  CHECK(pB.Eq("/C2[1]/GC2/@B"));
  CHECK(pV2.Eq("/C2[1]/GC3/@V2"));
  CHECK(pV3.Eq("/C2[1]/GC3/@V3"));
  CHECK(dtRoot->GetVarFromPath(pS.Chars(), nullS).GetString().Eq(s));
  CHECK_EQUAL(dtRoot->GetVarFromPath(pU.Chars(), nullS).GetString().ULong(), u);
  CHECK_EQUAL(dtRoot->GetVarFromPath(pF.Chars(), nullS).GetString().Float(), f);
  CHECK_EQUAL(dtRoot->GetVarFromPath(pB.Chars(), nullS).GetString().Bool(), b);
  CHECK_EQUAL(dtRoot->GetVarFromPath(pV2.Chars(), nullS).GetString().Vec2<tF32>(), v2);
  CHECK_EQUAL(dtRoot->GetVarFromPath(pV3.Chars(), nullS).GetString().Vec3<tF32>(), v3);
}

TEST_FIXTURE(FDTSerialize,Base64) {
  cString compressedString;
  niDebugFmt(("... ORIGINAL STRING: %d bytes", _dtXML.size()));
  {
    Ptr<iDataTable> ptrDT = CreateDataTableFromXML(_dtXML.c_str(),_dtXML.size());
    CHECK(ptrDT.IsOK());
    compressedString = DataTableToCompressedString(ptrDT);
    CHECK(!compressedString.IsEmpty());
    niDebugFmt(("... COMPRESSED STRING: (%d)\n<{%s}>", compressedString.size(), compressedString));
    CHECK_EQUAL(_dtXML_Base64Compressed.size(), compressedString.size());
    CHECK_EQUAL(
        compressedString,
        _dtXML_Base64Compressed);
  }
  {
    Ptr<iDataTable> ptrDT = CreateDataTableFromCompressedString(compressedString.Chars());
    CHECK(ptrDT.IsOK());
    cString xmlString = DataTableToXML(ptrDT);
    niDebugFmt(("... XML STRING: (%d) %s", xmlString.size(), xmlString));
    CHECK_EQUAL(_dtXML, xmlString);
  }

  // Same data but all on one line with some random spaces and new lines added
  {
    compressedString = "hwcAAHjavZVr\r\nT9swFIb/iuVPoJVgJ\t81NaisxLhISaAiqMfYlMulpa8   2NI9spMPHjd5wGemEX TRr70tqvj31eP/ZxBqeVk+6JlEpYW4kFDOlJs1g8UbLqnD66Y7H4ZO6lGwszA0dHg4/CApkZ 3dSvwVMlZnZIKYFK3CuYDCmnh6PB2IjKTrVZEK\rVLoYqFcEY+Dull+793wIIk7bMo7B2EAYvi JIUDxvIeC9IoS1nYYz0MYUmS8CxBNc/jPIxXGkuzpL8KSHFmHrZyzngWRb12YR5mMQawXuh/ +D4lFj3gnj5DGe3xHm+1zvl5NQcj3ZW20kldfXneE \r\t \n e52ha8vwrV2wgt+v8dzqSYGqtHgLVbE CEZscG2h7uAsu6DO1TUoXHsJf8mVBVmWRmmEyBBn1E+zzCPpJyyPE88iQKBR7mFnLdY8zfs+ OGRhGnMPDYM5w6aHytM8554/rspzP3zAwzCIs7DXD9Isiv35RTEOh/8Jc0uJTPVySPsxJdq4 uS6s/I5ZOWPIGIQpSiXrolai8io62JXgA2MJ6qaxrlkUZaOUrGZtrC5L1VjMtq36NDNdCTWk rD0GuQAchXXLrpvdQZ/AVDTKUYJpn/w8Yp2ui4c5VIUBC67VlNZ12zCwBGNXB01qzFzUGn9e x20N3aB1wrjCSZ8k9Rdk0nUySsrG4C18HQ2QEa6LQnsfsU7R/OGG58P1Tg5XNxcb68v8rvtk 77zPHZ9nmAvMS2Fdal9cv7bMXyzzd7K8Y/hn7lbakI51U87Z86VuLFzA1D1faVk5MBe6/AYT Sm5BzubuTCrU2ivLgpCSG6h8JS3xTIcU+0dlCQqrx9cSCmxb8W9RN3FTvhSPXo0oGRs5Qzen iOJp3G4BV7hBbLdIDX2D6C7vaHBU+qkd9vOqbtyVdOWckgnYclv5HbCNl4+49hPkvzNr2EsJ DwW+h7PVkxPu4fvGIh4F4b4//rcu7sTDloe2/48dsP2tEvtjtf0A80hu6Q==";
    Ptr<iDataTable> ptrDT = CreateDataTableFromCompressedString(compressedString.Chars());
    CHECK(ptrDT.IsOK());
    cString xmlString = DataTableToXML(ptrDT);
    niDebugFmt(("... XML STRING: (%d) %s", xmlString.size(), xmlString));
    CHECK_EQUAL(_dtXML, xmlString);
  }

  // Introduce some crap in the base64
  {
    compressedString = "hwcAAHjavZZZZZVrT9swFIb/iuVPoJVgJ81NaisxLhISaAiqMfYlMulpa82NI9spMPHjd5wGemEX TRr70tqvj31eP/ZxBqeVk+6JlEpYW4kFDOlJs1g8UbLqnD66Y7H4ZO6lGwszA0dHg4/CApkZ 3dSvwVMlZnZIKYFK3CuYDCmnh6PB2IjKTrVZEKVLoYqFcEY+Dull+793wIIk7bMo7B2EAYvi JIUDxvIeC9IoS1nYYz0MYUmS8CxBNc/jPIxXGkuzpL8KSHFmHrZyzngWRb12YR5mMQawXuh/ +D4lFj3gnj5DGe3xHm+1zvl5NQcj3ZW20kldfXneEe52ha8vwrV2wgt+v8dzqSYGqtHgLVbE CEZscG2h7uAsu6DO1TUoXHsJf8mVBVmWRmmEyBBn1E+zzCPpJyyPE88iQKBR7mFnLdY8zfs+ OGRhGnMPDYM5w6aHytM8554/rspzP3zAwzCIs7DXD9Isiv35RTEOh/8Jc0uJTPVySPsxJdq4 uS6s/I5ZOWPIGIQpSiXrolai8io62JXgA2MJ6qaxrlkUZaOUrGZtrC5L1VjMtq36NDNdCTWk rD0GuQAchXXLrpvdQZ/AVDTKUYJpn/w8Yp2ui4c5VIUBC67VlNZ12zCwBGNXB01qzFzUGn9e x20N3aB1wrjCSZ8k9Rdk0nUySsrG4C18HQ2QEa6LQnsfsU7R/OGG58P1Tg5XNxcb68v8rvtk 77zPHZ9nmAvMS2Fdal9cv7bMXyzzd7K8Y/hn7lbakI51U87Z86VuLFzA1D1faVk5MBe6/AYT Sm5BzubuTCrU2ivLgpCSG6h8JS3xTIcU+0dlCQqrx9cSCmxb8W9RN3FTvhSPXo0oGRs5Qzen iOJp3G4BV7hBbLdIDX2D6C7vaHBU+qkd9vOqbtyVdOWckgnYclv5HbCNl4+49hPkvzNr2EsJ DwW+h7PVkxPu4fvGIh4F4b4//rcu7sTDloe2/48dsP2tEvtjtf0A80hu6Q==";
    Ptr<iDataTable> ptrDT = CreateDataTableFromCompressedString(compressedString.Chars());
    // Expect failure
    CHECK(!ptrDT.IsOK());
    cString xmlString = DataTableToXML(ptrDT);
    niDebugFmt(("... XML STRING: (%d) %s", xmlString.size(), xmlString));
    CHECK_EQUAL(_ASTR(""), xmlString);
  }
}

TEST_FIXTURE(FDTSerialize,ReadXMLPropertyWithSpace) {
  Ptr<iDataTable> ptrDT = CreateDataTableFromXML(_dtXML_Prop_with_space.c_str(),_dtXML_Prop_with_space.size());
  CHECK(ptrDT.IsOK());

  CHECK_EQUAL(123, ptrDT->GetInt("Bytecollider"));
  CHECK_EQUAL(456, ptrDT->GetInt("Byte Collider"));
  CHECK_EQUAL(789, ptrDT->GetInt("ByteCollider"));
}
