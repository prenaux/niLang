// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
::Import("oo.ni")
::Import("sexp.ni")
::Import("assert.ni")
::Import("fs.ni")

module <- {
  _indent = "\t"

  tNode = {
    _name = ""
    _text = ""
    _kind = "element"
    _attributes = {}
    _children = []

    _top = null
    _stack = []

    function new(aName) this {
      local that = this.DeepClone()
      that._name = aName || "XMLNode";
      return that
    }

    function setAttribute(aName,aValue) {
      local node = _top || this
      node._attributes[aName] <- ""+aValue
    }

    function getAttribute(aName,_aDefault) {
      local node = _top || this
      if (!(aName in node._attributes))
        return _aDefault;
      return node._attributes[aName];
    }

    function addChild(aChildNode) {
      local node = _top || this
      node._children.push(aChildNode)
    }

    function pushNew(aName,_aKind) {
      local newNode = ::xml.tNode.new(aName);
      if (_aKind) {
        newNode._kind = _aKind;
      }
      local node = _top || this
      node._children.push(newNode);
      _stack.push(newNode);
      _top = newNode
      return newNode
    }

    function pushChild(aIndex) {
      local node = _top || this
      local childNode = node._children[aIndex]
      _stack.push(childNode);
      _top = childNode
      return childNode
    }

    function pop() {
      _stack.pop()
      _top = _stack.empty() ? null : _stack.top()
      return _top
    }

    function top() {
      return _top || this
    }
  }

  function parseXml(aFileOrString) {
    local sink = {
      _root = ::xml.tNode.new("XMLRootNode")
      function OnXmlParserSink_Node(aType, aNameOrText) {
        // ::println("NODE:" ::eXmlParserNodeType.gettable().getkey(aType) ":" aNameOrText)
        switch (aType) {
          case ::eXmlParserNodeType.ElementBegin: {
            _root.pushNew(aNameOrText,null)
            break;
          }
          case ::eXmlParserNodeType.ElementEnd: {
            _root.pop()
            break;
          }
          case ::eXmlParserNodeType.Text: {
            _root.pushNew("__innerText__","text")
            _root.top()._text = aNameOrText.decodexml();
            _root.pop()
            break;
          }
        }
        return true;
      }
      function OnXmlParserSink_Attribute(aName, aValue) {
        // ::println("ATTR:" aName "=" aValue)
        _root.setAttribute(aName,aValue.decodexml())
        return true;
      }
    }
    if (typeof aFileOrString == "string") {
      if (!::gLang.XmlParseString(aFileOrString,sink))
        throw "Xml Parser error"
    }
    else if (typeof aFileOrString == "iunknown") {
      if (!::gLang.XmlParseFile(aFileOrString.QueryInterface("iFile"),sink))
        throw "Xml Parser error"
    }
    else {
      throw "Invalid input type: " + typeof aFileOrString
    }
    if (sink._root._children.empty())
      throw "Empty Xml"
    return sink._root._children[?0];
  }

  function writeXmlIndent(fp,indent) {
    local w = 0;
    if (indent >= 0) {
      w += fp.WriteString("\n")
      foreach (i in indent) {
        w += fp.WriteString(_indent)
      }
    }
    return w
  }

  function _writeXmlToFile(fp,aNode,indent,aParent) {
    local nextIndent = (indent >= 0) ? (indent+1) : (-1);
    local w = 0;
    if (aParent) {
      w += writeXmlIndent(fp,indent)
    }
    // text node, write it...
    if (aNode._kind == "text") {
      w += fp.WriteString(aNode._text.encodexml())
      return w;
    }
    // element node...
    w += fp.WriteString("<")
    w += fp.WriteString(aNode._name)
    // write attributes
    local numAttrs = aNode._attributes.len();
    local propNL = (indent >= 0) && (numAttrs >= 4);

    local partial = []
    foreach (attrName,attrVal in aNode._attributes) {
      partial.push(attrName + "=\"" + attrVal.encodexml() + "\"")
    }
    partial.sort()

    foreach (p in partial) {
      if (propNL) {
        w += writeXmlIndent(fp,nextIndent)
      }
      else {
        w += fp.WriteString(" ")
      }
      w += fp.WriteString(p);
    }

    if (aNode._children.len() == 0) {
      w += fp.WriteString("/>");
    }
    else {
      w += fp.WriteString(">")
      local singleTextChild = (aNode._children.len() == 1 &&
                               aNode._children[0]._kind == "text")
      if (singleTextChild) {
        w += _writeXmlToFile(fp,aNode._children[0],nextIndent,null)
      }
      else {
        foreach (child in aNode._children) {
          w += _writeXmlToFile(fp,child,nextIndent,aNode)
        }
        w += writeXmlIndent(fp,indent)
      }
      w += fp.WriteString("</")
      w += fp.WriteString(aNode._name)
      w += fp.WriteString(">")
    }
    return w
  }

  function toFile(aNode,fp,aIndent,aParent) {
    if (typeof(fp) == "string") {
      local path = fp
      fp = ::fs.fileOpenWrite(fp)
      if (!fp)
        throw "Can't open output file:" + path
    }
    return _writeXmlToFile(fp,aNode,(aIndent == null) ? -1 : aIndent,aParent)
  }

  function toString(aNode,aIndent,aParent) {
    local fp = ::gLang.CreateFileDynamicMemory(0x1000,"--xmlstring--")
    _writeXmlToFile(fp,aNode,aIndent,aParent)
    fp.SeekSet(0)
    return fp.ReadString()
  }
}

::LINT_CHECK_TYPE("null", ::?xml);
::xml <- module
