// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
::Import("lang.ni")
::Import("fs.ni")
::Import("xml.ni")
::Import("assert.ni")

::error <- function(str) {
  ::println(str)
  ::gLang.Exit(1)
}

::fileOpen <- function(aPath) {
  local fp = ::fs.fileOpenRead(aPath)
  if (!fp) throw "Can't open file '"+aPath+"'."
  return fp
}

local strRootNode, desc, def
local args = ::GetArgs();
if (args.GetSize() == 4) {
  strRootNode = args[1]
  desc = args[2]
  def  = args[3]
}
else {
  ::error("syntax: MergeXML rootNodeName _MODNAME_Desc.xml _MODNAME_ModuleDef.xml")
}

function fopenRootNode(strFile,rootName) : (def) {
  if (!::fs.fileExists(strFile)) {
    strFile = strFile.adddirfront(def.getdir())
    if (!::fs.fileExists(strFile)) {
      strFile = def.getdir() + "_idl/" + strFile.getfile()
    }
  }
  local rootNode = ::xml.parseXml(::fileOpen(strFile))
  if (rootNode._name != rootName)
    return null
  return rootNode
}

function mergeIncludeXML(rootNode,strFile,strRootNode) {
  ::print("- Merging ["+strFile+"]: ")
  try {
    local newDocRootNode
    try {
      newDocRootNode = fopenRootNode(strFile,strRootNode)
      if (!newDocRootNode) {
        throw "Document doesnt have '"+strRootNode+"' as root node."
      }
    }
    catch (e) {
      throw "Can't open root node '"+strRootNode+"' from '"+strFile+"'."
    }
    foreach (c in newDocRootNode._children) {
      local toMerge = c.DeepClone()
      toMerge.setAttribute("source",strFile)
      rootNode.addChild(toMerge)
    }
  }
  catch (e) {
    ::error("Error: " + e)
  }
  ::println("Done.")
}

function mergeModuleXML(strRootNode,rootNode,docRootNode,processInclude) {
  rootNode._name = strRootNode
  rootNode._attributes = docRootNode._attributes.Clone()

  // Namespaces
  {
    foreach (c in docRootNode._children) {
      if (c._name == "namespace")
        rootNode.addChild(c.DeepClone())
    }
  }
  // Dependencies
  {
    rootNode.pushNew("dependencies")
    foreach (c in docRootNode._children) {
      if (c._name == "dependency")
        rootNode.addChild(c.DeepClone())
    }
    rootNode.pop()
  }
  // Object types
  {
    rootNode.pushNew("objecttypes")
    foreach (c in docRootNode._children) {
      if (c._name == "objecttype")
        rootNode.addChild(c.DeepClone())
    }
    rootNode.pop()
  }
  // Includes
  {
    rootNode.pushNew("includes")
    foreach (c in docRootNode._children) {
      if (c._name == "include")
        rootNode.addChild(c.DeepClone())
    }
    rootNode.pop()
  }

  // Process includes
  if (processInclude) {
    foreach (c in docRootNode._children) {
      if (c._name == "include") {
        local strFile = c._children[0]._text
        if (!strFile.endswith("xml"))
          strFile += ".xml";
        mergeIncludeXML(rootNode,strFile,strRootNode)
      }
    }
  }
}

::println("Merging["+strRootNode+"]: "+desc+" -> "+def)
::doc <- ::xml.parseXml(::fileOpen(desc))
// ::println("THEXML:" ::xml.toString(::doc,1))
::dst <- ::xml.tNode.new(strRootNode)
local rootNode = ::dst
local docRoot = ::doc
::assertEquals("module",docRoot._name,"Invalid ModuleDef root node")
mergeModuleXML(strRootNode,rootNode,docRoot,true)
// ::println("---RESULT---\n" ::xml.toString(::dst))
::xml.toFile(::dst,def,0)
