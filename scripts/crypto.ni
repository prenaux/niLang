// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

::Import("lang.ni")

::gCrypto <- ::CreateInstance("niLang.Crypto")

::crypto <- {
  function getHashObject(aHashType) {
    switch (typeof aHashType) {
      case "string": {
        return ::CreateInstance("niLang.CryptoHash",aHashType);
      }
      case "iunknown": {
        return aHashType
      }
      default: {
        throw "Invalid HashType '" + aHashType + "'."
      }
    }
  }

  //! Create a hash object
  function createHash(aHashType) {
    return getHashObject(aHashType)
  }

  //! Hash the specified file.
  function hashFile(aHashType, aFile) {
    local hash = getHashObject(aHashType)
    aFile.SeekSet(0)
    hash.Update(aFile,aFile.size)
    return hash
  }

  //! Hash the specified string.
  function hashString(aHashType, aString) {
    local hash = getHashObject(aHashType)
    local file = ::gLang.CreateFileDynamicMemory(1024, "");
    file.WriteString(aString);
    file.SeekSet(0);
    hash.Update(file, file.size);
    return hash
  }

  //! Hash the specified datatable
  function hashDataTable(aHashType, aDT) {
    local hash = getHashObject(aHashType)
    local file = ::gLang.CreateFileDynamicMemory(1024, "");
    if (!::gLang.SerializeDataTable("dtb",::eSerializeMode.Write,aDT,file))
      throw "Can't write data table to temporary hash file."
    file.SeekSet(0);
    hash.Update(file, file.size);
    return hash
  }
}
