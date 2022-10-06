<?xml version="1.0" encoding="UTF-8" ?>
<!-- (c) 2022 The niLang Authors -->

<!DOCTYPE stylesheet [
  <!ENTITY SAFEIDLNAME "translate(/niIDL/@name,'.-','__')">
  <!ENTITY PARAMETERS "parameter[not(@deprecated='yes')]">
  <!ENTITY METHODS "method[not(contains(@name,'operator')) and not(@name=../@name) and not(@name='niDeclareInterface') and not(@deprecated='yes') and not(@deprecated='yes')]">
  <!ENTITY PARENTS "parents/parent[not(@deprecated='yes') and not(../@deprecated='yes')]">
  <!ENTITY INTERFACES "/niIDL/namespace/interface[not(@noautomation='yes') and not(@deprecated='yes')]">
  <!ENTITY INCS "/niIDL/includes/include">
  <!ENTITY ALLINTERFACES "/niIDL/namespace/interface[not(@deprecated='yes')]">
  <!ENTITY CONSTANTS "/niIDL/namespace/variable[not(@noautomation='yes') and not(@deprecated='yes') and starts-with(@name,'k')]">
  <!ENTITY EMPTYENUMVALUES "/niIDL/namespace/enum[(@name='') and not(@noautomation='yes') and not(@deprecated='yes')]/value">
  <!ENTITY NAMEDENUMS "/niIDL/namespace/enum[not(@name='') and not(@noautomation='yes') and not(@deprecated='yes') and not(count(value)=0)]">
  <!ENTITY ALLENUMS "/niIDL/namespace/enum[not(@noautomation='yes') and not(@deprecated='yes') and not(count(value)=0)]">
  <!ENTITY OBJECTTYPES "/niIDL/objecttypes/objecttype">
  <!ENTITY DISPATCHWRAPPER "((@dispatchwrapper='yes') or (@dispatchwrappercreate) or (starts-with(@name,'i') and contains(@name,'Sink') and not(@name='iSinkList')))">
  <!ENTITY DEPENDENCIES "/niIDL/dependencies/dependency">
  <!ENTITY IDDEPENDENCIES "/niIDL/dependencies/dependency[not(@weak='yes') and not(@noid='yes')]">
  <!ENTITY AISPTR "(contains($A,'*') or contains($A,'&amp;') or contains($A,'Ptr&lt;'))">
  <!ENTITY AISINTERFACE "(starts-with($A,'i') or starts-with($A,'const i') or contains($A,'::i')) and not(starts-with($A,'int')) or contains($A,'Ptr&lt;')">
  <!ENTITY AISENUM "(starts-with($A,'e') or starts-with($A,'const e') or contains($A,'::e'))">
  <!ENTITY AISFLAGS "(((starts-with($A,'t') or starts-with($A,'const t')) and contains($A,'Flags')) or (contains($A,'::t') and contains($A,'Flags')))">
  <!ENTITY MATCH_VAR "starts-with($A,'Var') or starts-with($A,'const Var') or starts-with($A,'ni::Var') or starts-with($A,'const ni::Var')">
  <!ENTITY AISVEC "(contains($A,'CVec'))">
  <!ENTITY AISSINKLIST "(contains($A,'iSinkLst') or contains($A,'SinkLst'))">
  <!ENTITY AISLST "(contains($A,'CLst') or contains($A,'SinkLst'))">
  <!ENTITY AISMAP "(contains($A,'CMap'))">
  <!ENTITY AISCOLLECTION "(contains($A,'CVec')) or (contains($A,'CLst') or contains($A,'SinkLst')) or (contains($A,'CMap'))">
]>

<xsl:stylesheet version="1.0"
        xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
        xmlns:math="http://exslt.org/math"
        xmlns:date="http://exslt.org/dates-and-times">

  <xsl:include href="GenModuleBase.xslt"/>
  <xsl:output method="text" />

<!-- ROOT ****************************************************** -->
  <xsl:template match="/">

    <xsl:if test="not(/niIDL/@name)">
      <xsl:message terminate="yes">E/ Unnamed IDL cannot be used to generate automation wrapper !</xsl:message>
    </xsl:if>

    <!-- <xsl:if test="not(/niIDL/namespace/@name)"> -->
      <!-- <xsl:message terminate="yes">E/ No namespace defined !</xsl:message> -->
    <!-- </xsl:if> -->

    <!-- xsl:if test="count(&OBJECTTYPES;)=0">
      <xsl:message terminate="yes">No object types defined !</xsl:message>
    </xsl:if -->

    <xsl:call-template name="write_header"/>

    <xsl:call-template name="newline"/>
    <xsl:text>namespace </xsl:text><xsl:value-of select="/niIDL/namespace/@name"/><xsl:text> {</xsl:text>

    <xsl:call-template name="get_interfaces_wrappers"/>

    <xsl:if test="not(count(&ALLENUMS;)=0)"><xsl:call-template name="get_enumerations"/></xsl:if>

    <xsl:call-template name="newline"/>
    <xsl:if test="not((count(&CONSTANTS;)+count(&INTERFACES;))=0)"><xsl:call-template name="get_constants"/></xsl:if>
    <xsl:call-template name="newline"/>

<xsl:text>
//////////////////////////////////////////////////////////////////////////////////////////////
// GetModuleDef
//////////////////////////////////////////////////////////////////////////////////////////////
</xsl:text>

    <xsl:text>niExportFunc(const ni::iModuleDef*) GetModuleDef_</xsl:text><xsl:value-of select="&SAFEIDLNAME;"/><xsl:text>()
{
  static bool _initialized = false;
  if (!_initialized) {
    _initialized = true;
    niModuleDefImpl_GetModule_Register();
  }
</xsl:text>
  <xsl:call-template name="get_objecttypes"/>
  <xsl:call-template name="get_module"/>
  <xsl:text>
  return &amp;ModuleDef;
} // End of GetModuleDef_</xsl:text><xsl:value-of select="&SAFEIDLNAME;"/><xsl:call-template name="newline"/>
<xsl:text>} // end of namespace </xsl:text><xsl:value-of select="/niIDL/namespace/@name"/><xsl:call-template name="newline"/>

    <xsl:call-template name="write_footer"/>
  </xsl:template>

<!-- ************************************************************ -->
  <xsl:template name="write_header">
  <xsl:text>//////////////////////////////////////////////////////////////////////////////////////////////
//
// Autogenerated module definition, for </xsl:text><xsl:value-of select="/niIDL/namespace/@name"/>::<xsl:value-of select="/niIDL/@name"/><xsl:text>.
//
//////////////////////////////////////////////////////////////////////////////////////////////
//lint -e750 : local macro '_XXX_ModuleDef' (line NN, file _XXX_ModuleDef.cpp) not referenced
//lint -e766 : Header file '...\ModuleDefImpl_Begin.h' not used in module '_XXX_ModuleDef.cpp'
</xsl:text>
<xsl:if test="/niIDL/@moduleinclude">
<xsl:if test="string-length(/niIDL/@moduleinclude)=0">
#include &lt;niLang/Types.h&gt;
#include &lt;niLang/Utils/ObjModelImpl.h&gt;
</xsl:if>
<xsl:if test="not(string-length(/niIDL/@moduleinclude)=0)">
#include "<xsl:value-of select="/niIDL/@moduleinclude"/>"
</xsl:if>
</xsl:if>
<xsl:if test="not(/niIDL/@moduleinclude)">
#include "stdafx.h"
</xsl:if>
<xsl:text>#include "API/</xsl:text><xsl:value-of select="/niIDL/@name"/><xsl:text>_ModuleDef.h"
#include &lt;niLang/Utils/ModuleDefImpl_Begin.h&gt;
</xsl:text>

  <xsl:if test="/niIDL/@interfacesheaderpath">
    <xsl:text>#include "</xsl:text><xsl:value-of select="/niIDL/@interfacesheaderpath"/><xsl:text>&quot;</xsl:text><xsl:call-template name="newline"/>
  </xsl:if>

  <xsl:call-template name="newline"/>
  <xsl:text>#ifndef niConfig_OnlyObjectTypesIDL</xsl:text>
  <xsl:call-template name="newline"/>
  <xsl:if test="not(count(&IDDEPENDENCIES;)=0)">
  <xsl:for-each select="&IDDEPENDENCIES;">
    <xsl:choose>
      <xsl:when test="@includepath">
        <xsl:text>#include &lt;</xsl:text><xsl:value-of select="@includepath"/><xsl:text>&gt;</xsl:text><xsl:call-template name="newline"/>
      </xsl:when>
    </xsl:choose>
  </xsl:for-each>
  </xsl:if>
  <xsl:text>#endif // #ifndef niConfig_OnlyObjectTypesIDL</xsl:text>
  <xsl:call-template name="newline"/>

  <xsl:choose>
    <xsl:when test="/niIDL/@idheaderpath">
      <xsl:text>#include &lt;</xsl:text><xsl:value-of select="/niIDL/@idheaderpath"/><xsl:text>.h&gt;</xsl:text><xsl:call-template name="newline"/>
    </xsl:when>
  </xsl:choose>
  <xsl:call-template name="newline"/>

  <xsl:text>

#if !defined niConfig_OnlyObjectTypesIDL &amp;&amp; !defined niConfig_NoInterfaceDef
#define _</xsl:text><xsl:value-of select="&SAFEIDLNAME;"/><xsl:text>_ModuleDef
#include &lt;niLang/Utils/VMCallCImpl.h&gt;
#ifndef niConfig_NoXCALL
#define IDLC_BEGIN_NAMESPACE() namespace </xsl:text><xsl:value-of select="/niIDL/namespace/@name"/><xsl:text> { namespace xcall_cimpl {
#define IDLC_END_NAMESPACE() }}
#ifndef XCALL_CIMPL
#define XCALL_CIMPL(NAME) </xsl:text><xsl:value-of select="/niIDL/namespace/@name"/><xsl:text>::xcall_cimpl::NAME
#endif
</xsl:text>
  <xsl:for-each select="&INCS;">
    <xsl:text>#include "</xsl:text>
        <xsl:choose>
            <xsl:when test="contains(.,'/_idl/')">
                <xsl:text>_idl/</xsl:text>
                <xsl:value-of select="substring-after(substring-before(.,'.xml'),'/_idl/')" />
            </xsl:when>
            <xsl:when test="contains(.,'API/')">
                <xsl:text>_idl/</xsl:text>
                <xsl:call-template name="substring-after-last">
                    <xsl:with-param name="string" select="substring-before(.,'.xml')" />
                    <xsl:with-param name="delimiter" select="'/'" />
                </xsl:call-template>
            </xsl:when>
            <xsl:when test="contains(.,'include/')">
                <xsl:value-of select="substring-after(substring-before(.,'.xml'),'include/')" />
            </xsl:when>
            <xsl:otherwise>
                <xsl:value-of select="substring-before(.,'.xml')" />
            </xsl:otherwise>
        </xsl:choose>
    <xsl:text>.inl"
</xsl:text>
  </xsl:for-each>
    <xsl:text>#endif // niConfig_NoXCALL
#endif // #if !defined niConfig_OnlyObjectTypesIDL &amp;&amp; !defined niConfig_NoInterfaceDef
</xsl:text>
  </xsl:template>

<!-- ************************************************************ -->
  <xsl:template name="write_footer">
<xsl:text>
/// EOF //////////////////////////////////////////////////////////////////////////////////////
#include &lt;niLang/Utils/ModuleDefImpl_End.h&gt;
</xsl:text>
  </xsl:template>

<!-- ************************************************************ -->
  <xsl:template name="get_module">
<xsl:text>
//////////////////////////////////////////////////////////////////////////////////////////////
// Module definition
//////////////////////////////////////////////////////////////////////////////////////////////
</xsl:text>

<!-- DEPENDENCIES ARRAY -->
<xsl:if test="not(count(&DEPENDENCIES;)=0)">
<xsl:text>static const ni::achar* Dependencies[] = {</xsl:text><xsl:call-template name="newline"/>
<xsl:for-each select="&DEPENDENCIES;">
  <xsl:call-template name="tab"/><xsl:text>&quot;</xsl:text><xsl:value-of select="@name"/><xsl:text>",</xsl:text><xsl:call-template name="newline"/>
</xsl:for-each>
<xsl:text>};</xsl:text><xsl:call-template name="newline"/>
</xsl:if>

<!-- OBJECT TYPES ARRAY -->
<xsl:if test="not(count(&OBJECTTYPES;)=0)">
<xsl:text>static const ni::sObjectTypeDef* ObjectTypes[] = {</xsl:text><xsl:call-template name="newline"/>
<xsl:for-each select="&OBJECTTYPES;">
    <xsl:call-template name="get_min_features_begin" />
  <xsl:call-template name="tab"/><xsl:text>&amp;ObjectType_</xsl:text><xsl:value-of select="@category"/>_<xsl:value-of select="@name"/><xsl:text>,</xsl:text><xsl:call-template name="newline"/>
    <xsl:call-template name="get_min_features_end" />
</xsl:for-each>
<xsl:text>};</xsl:text><xsl:call-template name="newline"/>
</xsl:if>

<!-- INTERFACES ARRAY -->
<xsl:call-template name="newline"/>
<xsl:text>#if !defined niConfig_OnlyObjectTypesIDL &amp;&amp; !defined niConfig_NoInterfaceDef</xsl:text>
<xsl:call-template name="newline"/>
<xsl:if test="not(count(&INTERFACES;)=0)">
<xsl:text>static const ni::sInterfaceDef* Interfaces[] = {</xsl:text><xsl:call-template name="newline"/>
<xsl:for-each select="&INTERFACES;">
    <xsl:call-template name="get_min_features_begin" />
  <xsl:call-template name="tab"/><xsl:text>GetInterfaceDef_</xsl:text><xsl:value-of select="@name"/><xsl:text>(),</xsl:text><xsl:call-template name="newline"/>
    <xsl:call-template name="get_min_features_end" />
</xsl:for-each>
<xsl:text>};
</xsl:text>
</xsl:if>
<xsl:call-template name="newline"/>
<xsl:text>#endif // #if !defined niConfig_OnlyObjectTypesIDL &amp;&amp; !defined niConfig_NoInterfaceDef</xsl:text>
<xsl:call-template name="newline"/>

<!-- ENUMERATIONS ARRAY -->
<xsl:if test="not(count(&ALLENUMS;)=0)">
    <xsl:call-template name="newline"/>
    <xsl:text>#if !defined niConfig_OnlyObjectTypesIDL &amp;&amp; !defined niConfig_NoEnumDef</xsl:text>
    <xsl:call-template name="newline"/>
  <xsl:text>static const ni::sEnumDef* Enumerations[] = {</xsl:text><xsl:call-template name="newline"/>
  <xsl:if test="not(count(&EMPTYENUMVALUES;)=0)">
    <xsl:call-template name="tab"/><xsl:text>GetEnumDef_Unnamed(),</xsl:text><xsl:call-template name="newline"/>
  </xsl:if>
  <xsl:for-each select="&NAMEDENUMS;">
        <xsl:call-template name="get_min_features_begin" />
    <xsl:call-template name="tab"/><xsl:text>GetEnumDef_</xsl:text><xsl:value-of select="@name"/><xsl:text>(),</xsl:text><xsl:call-template name="newline"/>
        <xsl:call-template name="get_min_features_end" />
  </xsl:for-each>
  <xsl:text>};</xsl:text><xsl:call-template name="newline"/>
    <xsl:call-template name="newline"/>
    <xsl:text>#endif // #if !defined niConfig_OnlyObjectTypesIDL &amp;&amp; !defined niConfig_NoEnumDef</xsl:text>
    <xsl:call-template name="newline"/>
</xsl:if>

<!-- CONSTANTS ARRAY -->
<xsl:if test="not((count(&CONSTANTS;))=0)">
<xsl:call-template name="newline"/>
<xsl:text>#if !defined niConfig_OnlyObjectTypesIDL &amp;&amp; !defined niConfig_NoConstDef</xsl:text>
<xsl:call-template name="newline"/>
<xsl:text>static const ni::sConstantDef* Constants[] = {</xsl:text><xsl:call-template name="newline"/>
<xsl:for-each select="&CONSTANTS;">
    <xsl:call-template name="get_min_features_begin" />
  <xsl:call-template name="tab"/><xsl:text>&amp;ConstantDef_</xsl:text><xsl:value-of select="@name"/><xsl:text>,</xsl:text><xsl:call-template name="newline"/>
    <xsl:call-template name="get_min_features_end" />
</xsl:for-each>
<xsl:text>};</xsl:text><xsl:call-template name="newline"/>
<xsl:call-template name="newline"/>
<xsl:text>#endif // #if !defined niConfig_OnlyObjectTypesIDL &amp;&amp; !defined niConfig_NoConstDef</xsl:text>
<xsl:call-template name="newline"/>
</xsl:if>

<xsl:text>
static const ni::sModuleDef ModuleDef(
</xsl:text>
<xsl:call-template name="tab"/><xsl:text>&quot;</xsl:text><xsl:value-of select="/niIDL/@name"/><xsl:text>",</xsl:text><xsl:call-template name="newline"/>
<xsl:call-template name="tab"/><xsl:text>&quot;</xsl:text><xsl:value-of select="/niIDL/@version"/><xsl:text>",</xsl:text><xsl:call-template name="newline"/>
<xsl:call-template name="tab"/><xsl:text>&quot;</xsl:text><xsl:value-of select="/niIDL/@desc"/><xsl:text> (Build Date: " __DATE__ ")",</xsl:text><xsl:call-template name="newline"/>
<xsl:call-template name="tab"/><xsl:text>&quot;</xsl:text><xsl:value-of select="/niIDL/@author"/><xsl:text>",</xsl:text><xsl:call-template name="newline"/>
<xsl:call-template name="tab"/><xsl:text>&quot;</xsl:text><xsl:value-of select="/niIDL/@copyright"/><xsl:text>",</xsl:text><xsl:call-template name="newline"/>

<xsl:call-template name="tab"/><xsl:value-of select="count(&DEPENDENCIES;)"/>
<xsl:choose>
  <xsl:when test="not(count(&DEPENDENCIES;)=0)"><xsl:text>, Dependencies,</xsl:text></xsl:when>
  <xsl:otherwise><xsl:text>, NULL,</xsl:text></xsl:otherwise>
</xsl:choose>

<xsl:call-template name="newline"/>
<xsl:text>#if !defined niConfig_OnlyObjectTypesIDL &amp;&amp; !defined niConfig_NoInterfaceDef</xsl:text>
<xsl:call-template name="newline"/>
<xsl:call-template name="tab"/>
<xsl:choose>
  <xsl:when test="not((count(&INTERFACES;))=0)"><xsl:text>niCountOf(Interfaces), Interfaces,</xsl:text></xsl:when>
  <xsl:otherwise><xsl:text>0, NULL,</xsl:text></xsl:otherwise>
</xsl:choose>
<xsl:call-template name="newline"/>
<xsl:text>#else
  0, NULL,
#endif</xsl:text>

<xsl:call-template name="newline"/>
<xsl:text>#if !defined niConfig_OnlyObjectTypesIDL &amp;&amp; !defined niConfig_NoEnumDef</xsl:text>
<xsl:call-template name="newline"/>
<xsl:choose>
  <xsl:when test="not(count(&ALLENUMS;)=0)">
    <xsl:call-template name="tab"/>
        <xsl:text>niCountOf(Enumerations)</xsl:text>
    <xsl:text>, Enumerations,</xsl:text><xsl:call-template name="newline"/>
  </xsl:when>
  <xsl:otherwise>
  <xsl:text>  0, NULL,
</xsl:text>
  </xsl:otherwise>
</xsl:choose>
<xsl:text>#else
  0, NULL,
#endif</xsl:text>

<xsl:call-template name="newline"/>
<xsl:text>#if !defined niConfig_OnlyObjectTypesIDL &amp;&amp; !defined niConfig_NoConstDef</xsl:text>
<xsl:call-template name="newline"/>
<xsl:call-template name="tab"/>
<xsl:choose>
  <xsl:when test="not((count(&CONSTANTS;))=0)"><xsl:text>niCountOf(Constants), Constants,</xsl:text></xsl:when>
  <xsl:otherwise><xsl:text>0, NULL,</xsl:text></xsl:otherwise>
</xsl:choose>
<xsl:call-template name="newline"/>
<xsl:text>#else
  0, NULL,
#endif</xsl:text>

<xsl:call-template name="newline"/>
<xsl:call-template name="tab"/>
<xsl:choose>
  <xsl:when test="not(count(&OBJECTTYPES;)=0)">
        <xsl:text>niCountOf(ObjectTypes), ObjectTypes</xsl:text>
    </xsl:when>
  <xsl:otherwise>
        <xsl:text>0, NULL</xsl:text>
    </xsl:otherwise>
</xsl:choose>
<xsl:text>
);
</xsl:text>
  </xsl:template>

<!-- ************************************************************ -->
  <xsl:template name="get_objecttypes">
<xsl:text>
//////////////////////////////////////////////////////////////////////////////////////////////
// Object types definition
//////////////////////////////////////////////////////////////////////////////////////////////
</xsl:text>

      <xsl:for-each select="&OBJECTTYPES;">
          <xsl:call-template name="get_min_features_begin" />
          <xsl:text>static const ni::sObjectTypeDef ObjectType_</xsl:text>
          <xsl:value-of select="@category"/>_<xsl:value-of select="@name"/>
          <xsl:text>(</xsl:text>
          <xsl:text>&quot;</xsl:text>
          <xsl:choose>
              <xsl:when test="@category">
                  <xsl:value-of select="@category"/>
              </xsl:when>
              <xsl:otherwise>
                  <xsl:value-of select="/niIDL/@name"/>
              </xsl:otherwise>
          </xsl:choose>
          <xsl:text>.</xsl:text><xsl:value-of select="@name"/><xsl:text>&quot;,</xsl:text>
          <xsl:text>New_</xsl:text>
              <xsl:choose>
                  <xsl:when test="@category">
                      <xsl:value-of select="@category"/>
                  </xsl:when>
                  <xsl:otherwise>
                      <xsl:value-of select="&SAFEIDLNAME;"/>
                  </xsl:otherwise>
              </xsl:choose>
          <xsl:text>_</xsl:text><xsl:value-of select="@name"/>
          <xsl:text>);</xsl:text>
          <xsl:call-template name="newline"/>
      </xsl:for-each>

	<xsl:for-each select="&OBJECTTYPES;">
	</xsl:for-each>

	<xsl:call-template name="newline"/>

	</xsl:template>

<!-- ************************************************************ -->
	<xsl:template name="get_interfaces_wrappers">
		<xsl:call-template name="newline"/>
		<xsl:text>#if !defined niConfig_OnlyObjectTypesIDL &amp;&amp; !defined niConfig_NoInterfaceDef</xsl:text>
		<xsl:call-template name="newline"/>
		<xsl:for-each select="&INTERFACES;"><xsl:call-template name="get_interface_wrapper"/></xsl:for-each>
		<xsl:call-template name="newline"/>
		<xsl:text>#endif // #if !defined niConfig_OnlyObjectTypesIDL &amp;&amp; !defined niConfig_NoInterfaceDef</xsl:text>
		<xsl:call-template name="newline"/>
	</xsl:template>

<!-- ************************************************************ -->
	<xsl:template name="get_constants">
<xsl:text>

//////////////////////////////////////////////////////////////////////////////////////////////
// Constants definition
//////////////////////////////////////////////////////////////////////////////////////////////
#if !defined niConfig_OnlyObjectTypesIDL &amp;&amp; !defined niConfig_NoConstDef
</xsl:text>

	<xsl:for-each select="&CONSTANTS;">
        <xsl:call-template name="get_min_features_begin" />
		<xsl:text>static const ni::sConstantDef ConstantDef_</xsl:text><xsl:value-of select="@name"/><xsl:text> = ni::sConstantDef(</xsl:text><xsl:call-template name="newline"/>
		<xsl:call-template name="tab"/><xsl:text>&quot;</xsl:text><xsl:value-of select="@name"/><xsl:text>",</xsl:text><xsl:call-template name="newline"/>
		<xsl:call-template name="tab"/><xsl:text>ni::Var(</xsl:text><xsl:value-of select="@name"/><xsl:text>)</xsl:text><xsl:call-template name="newline"/>
		<xsl:text>);</xsl:text><xsl:call-template name="newline"/>
        <xsl:call-template name="get_min_features_end" />
	</xsl:for-each>

<xsl:call-template name="newline"/>
<xsl:text>#endif // #if !defined niConfig_OnlyObjectTypesIDL &amp;&amp; !defined niConfig_NoConstDef</xsl:text>
	</xsl:template>

<!-- ************************************************************ -->
	<xsl:template name="get_enumerations">
<xsl:text>

//////////////////////////////////////////////////////////////////////////////////////////////
// Enumerations definition
//////////////////////////////////////////////////////////////////////////////////////////////
#if !defined niConfig_OnlyObjectTypesIDL &amp;&amp; !defined niConfig_NoEnumDef
</xsl:text>

<xsl:if test="not(count(&EMPTYENUMVALUES;)=0)">
<xsl:text>
// --- Not named enums ---
niExportFunc(const ni::sEnumDef*) GetEnumDef_Unnamed() {

static const ni::sEnumValueDef Enum_Unnamed_Values[] = {</xsl:text><xsl:call-template name="newline"/>
	<xsl:for-each select="&EMPTYENUMVALUES;">
		<xsl:choose>
			<xsl:when test="starts-with(@name,'e')">
                <xsl:text>	{ "</xsl:text><xsl:value-of select="substring-after(@name,'e')"/><xsl:text>", </xsl:text><xsl:value-of select="/niIDL/namespace/@name"/><xsl:text>::</xsl:text><xsl:value-of select="@name"/><xsl:text> },</xsl:text>
			</xsl:when>
			<xsl:otherwise>
                <xsl:text>	{ "</xsl:text><xsl:value-of select="@name"/><xsl:text>", </xsl:text><xsl:value-of select="/niIDL/namespace/@name"/><xsl:text>::</xsl:text><xsl:value-of select="@name"/><xsl:text> },</xsl:text>
			</xsl:otherwise>
		</xsl:choose>
		<xsl:call-template name="newline"/>
	</xsl:for-each>
<xsl:text>};
static const ni::sEnumDef Enum_Unnamed = {
	"</xsl:text><xsl:value-of select="&SAFEIDLNAME;"/><xsl:text>.Unnamed",
	</xsl:text><xsl:value-of select="count(&EMPTYENUMVALUES;)"/><xsl:text>, Enum_Unnamed_Values
};

	return &amp;Enum_Unnamed;
}</xsl:text>

</xsl:if>

	<xsl:for-each select="&NAMEDENUMS;"><xsl:call-template name="get_named_enum_wrapper"/></xsl:for-each>

	<xsl:text>#endif // #if !defined niConfig_OnlyObjectTypesIDL &amp;&amp; !defined niConfig_NoEnumDef</xsl:text>
	</xsl:template>

<!-- ************************************************************ -->
	<xsl:template name="get_named_enum_wrapper">
	<xsl:text>
// --- </xsl:text><xsl:value-of select="@name"/><xsl:text> ---
</xsl:text><xsl:call-template name="get_min_features_begin" />
<xsl:text>niExportFunc(const ni::sEnumDef*) GetEnumDef_</xsl:text><xsl:value-of select="@name"/><xsl:text>() {

static const ni::sEnumValueDef Enum_</xsl:text><xsl:value-of select="@name"/><xsl:text>_Values[] = {</xsl:text><xsl:call-template name="newline"/>
	<xsl:for-each select="value">
		<xsl:if test="not(@deprecated='yes')">
        <xsl:call-template name="get_min_features_begin" />
		<xsl:choose>
			<xsl:when test="starts-with(@name,concat(../@name,'_'))">
                <xsl:text>	{ "</xsl:text><xsl:value-of select="substring-after(@name,concat(../@name,'_'))"/><xsl:text>", </xsl:text><xsl:value-of select="/niIDL/namespace/@name"/><xsl:text>::</xsl:text><xsl:value-of select="@name"/><xsl:text> },</xsl:text>
            </xsl:when>
			<xsl:otherwise>
                <xsl:text>	{ "</xsl:text><xsl:value-of select="@name"/><xsl:text>", </xsl:text><xsl:value-of select="/niIDL/namespace/@name"/><xsl:text>::</xsl:text><xsl:value-of select="@name"/><xsl:text> },</xsl:text>
            </xsl:otherwise>
		</xsl:choose>
        <xsl:call-template name="newline"/>
        <xsl:call-template name="get_min_features_end" />
		</xsl:if>
	</xsl:for-each>
	<xsl:text>};</xsl:text>
<xsl:text>
static const ni::sEnumDef Enum_</xsl:text><xsl:value-of select="@name"/><xsl:text> = {
	"</xsl:text><xsl:value-of select="@name"/><xsl:text>",</xsl:text><xsl:call-template name="newline"/>
	<xsl:call-template name="tab"/><xsl:text>niCountOf(Enum_</xsl:text><xsl:value-of select="@name"/><xsl:text>_Values), Enum_</xsl:text><xsl:value-of select="@name"/><xsl:text>_Values</xsl:text><xsl:call-template name="newline"/>
<xsl:text>};

	return &amp;Enum_</xsl:text><xsl:value-of select="@name"/><xsl:text>;
}</xsl:text>
    <xsl:call-template name="newline"/>
    <xsl:call-template name="get_min_features_end" />
	</xsl:template>

<!-- ************************************************************ -->
	<xsl:template name="get_interface_wrapper">
<xsl:text>

//////////////////////////////////////////////////////////////////////////////////////////////
</xsl:text>
<xsl:text>// </xsl:text><xsl:value-of select="@name"/><xsl:text> wrapper
</xsl:text>
<xsl:text>//////////////////////////////////////////////////////////////////////////////////////////////
</xsl:text>
    <xsl:call-template name="get_min_features_begin" />
		<xsl:text>niExportFunc(const ni::sInterfaceDef*) GetInterfaceDef_</xsl:text><xsl:value-of select="@name"/><xsl:text>() {</xsl:text>
		<xsl:call-template name="newline"/>
		<xsl:call-template name="newline"/>

		<xsl:if test="not(count(&METHODS;)=0)">
			<!-- METHODS -->
			<xsl:for-each select="&METHODS;">
                <xsl:call-template name="get_min_features_begin" />
				<xsl:call-template name="get_method_wrapper"><xsl:with-param name="A"><xsl:value-of select="../@name"/></xsl:with-param></xsl:call-template>
                <xsl:call-template name="get_min_features_end" />
                <xsl:call-template name="newline"/>
			</xsl:for-each>

			<!-- METHODS ARRAY -->
			<xsl:text>static const ni::sMethodDef* Methods_</xsl:text><xsl:value-of select="@name"/><xsl:text>[] = {</xsl:text>
			<xsl:call-template name="newline"/>

			<xsl:for-each select="&METHODS;">
                <xsl:call-template name="get_min_features_begin" />
				<xsl:text>	&amp;</xsl:text><xsl:value-of select="../@name"/><xsl:text>_</xsl:text><xsl:value-of select="@name"/><xsl:text>,</xsl:text><xsl:call-template name="newline"/>
                <xsl:call-template name="get_min_features_end" />
			</xsl:for-each>
			<xsl:call-template name="newline"/>

			<xsl:text>};</xsl:text>
			<xsl:call-template name="newline"/>
		</xsl:if>

		<!-- INTERFACE DISPATCH WRAPPER -->
		<xsl:if test="&DISPATCHWRAPPER;">
            <xsl:call-template name="newline"/>
            <xsl:if test="@dispatchwrappercreate">
                <xsl:text>//// Interface dispatch wrapper: </xsl:text><xsl:value-of select="@dispatchwrappercreate"/>
                <xsl:call-template name="newline"/>
            </xsl:if>
            <xsl:if test="not(@dispatchwrappercreate)">
                <xsl:text>//// Interface dispatch wrapper /////////////////////////////</xsl:text>
                <xsl:call-template name="newline"/>
                <xsl:call-template name="get_dispatch_wrapper"/>
            </xsl:if>
		</xsl:if>

		<!-- INTERFACE DESC -->
		<xsl:call-template name="newline"/>
		<xsl:text>//// Interface description //////////////////////////////////</xsl:text>
		<xsl:call-template name="newline"/>

		<xsl:if test="not(count(&PARENTS;)=0)">
			<xsl:text>static const ni::tUUID* Parents_</xsl:text><xsl:value-of select="@name"/><xsl:text>[] = {</xsl:text><xsl:call-template name="newline"/>
			<xsl:for-each select="&PARENTS;">
				<xsl:call-template name="tab"/>
				<xsl:text>&amp;niGetInterfaceUUID(</xsl:text><xsl:value-of select="@name"/><xsl:text>),</xsl:text>
				<xsl:call-template name="newline"/>
			</xsl:for-each>
			<xsl:text>};</xsl:text><xsl:call-template name="newline"/><xsl:call-template name="newline"/>
		</xsl:if>

		<xsl:text>static const ni::sInterfaceDef InterfaceDef_</xsl:text><xsl:value-of select="@name"/><xsl:text> = {</xsl:text><xsl:call-template name="newline"/>
		<xsl:call-template name="tab"/><xsl:text>niGetInterfaceID(</xsl:text><xsl:value-of select="@name"/><xsl:text>),</xsl:text><xsl:call-template name="newline"/>
		<xsl:call-template name="tab"/><xsl:text>&amp;niGetInterfaceUUID(</xsl:text><xsl:value-of select="@name"/><xsl:text>),</xsl:text><xsl:call-template name="newline"/>
		<xsl:call-template name="tab"/><xsl:value-of select="count(&PARENTS;)"/><xsl:text>,</xsl:text><xsl:call-template name="newline"/>
		<xsl:choose>
			<xsl:when test="not(count(&PARENTS;)=0)"><xsl:call-template name="tab"/><xsl:text>Parents_</xsl:text><xsl:value-of select="@name"/><xsl:text>,</xsl:text><xsl:call-template name="newline"/></xsl:when>
			<xsl:otherwise><xsl:call-template name="tab"/><xsl:text>NULL,</xsl:text><xsl:call-template name="newline"/></xsl:otherwise>
		</xsl:choose>
		<xsl:call-template name="tab"/>
		<xsl:choose>
			<xsl:when test="not(count(&METHODS;)=0)">
                <xsl:text>niCountOf(Methods_</xsl:text><xsl:value-of select="@name"/><xsl:text>),</xsl:text>
                <xsl:text>Methods_</xsl:text><xsl:value-of select="@name"/><xsl:text>,</xsl:text><xsl:call-template name="newline"/></xsl:when>
			<xsl:otherwise><xsl:text>0,NULL,</xsl:text><xsl:call-template name="newline"/></xsl:otherwise>
		</xsl:choose>
		<xsl:choose>

			<xsl:when test="&DISPATCHWRAPPER;">
                <xsl:call-template name="tab"/>
                <xsl:if test="@dispatchwrappercreate">
                    <xsl:value-of select="@dispatchwrappercreate"/>
                </xsl:if>
                <xsl:if test="not(@dispatchwrappercreate)">
                    <xsl:text>DispatchWrapper_</xsl:text><xsl:value-of select="@name"/><xsl:text>::Create</xsl:text>
                </xsl:if>
                <xsl:call-template name="newline"/>

            </xsl:when>
			<xsl:otherwise><xsl:call-template name="tab"/><xsl:text>NULL</xsl:text><xsl:call-template name="newline"/></xsl:otherwise>

		</xsl:choose>
		<xsl:text>};</xsl:text><xsl:call-template name="newline"/><xsl:call-template name="newline"/>

		<xsl:text>	return &amp;InterfaceDef_</xsl:text><xsl:value-of select="@name"/><xsl:text>;
}</xsl:text>
		<xsl:call-template name="newline"/><xsl:call-template name="newline"/>


		<xsl:text>// End of </xsl:text><xsl:value-of select="@name"/><xsl:text> wrapper</xsl:text>
    <xsl:call-template name="newline"/>
    <xsl:call-template name="get_min_features_end" />

	</xsl:template>

<!-- ************************************************************ -->
	<xsl:template name="get_method_wrapper">
		<xsl:param name="A" />

		<!-- PARAMETERS -->
		<xsl:text>// Method: </xsl:text><xsl:value-of select="@name"/>
		<xsl:call-template name="newline"/>

		<xsl:if test="not(count(&PARAMETERS;)=0)">
      <xsl:text>static const ni::sParameterDef </xsl:text><xsl:value-of select="$A"/><xsl:text>_</xsl:text><xsl:value-of select="@name"/><xsl:text>_Parameters[</xsl:text><xsl:value-of select="count(&PARAMETERS;)" /><xsl:text>] = { </xsl:text>
      <xsl:for-each select="&PARAMETERS;">
        <xsl:call-template name="newline"/>
        <xsl:call-template name="tab"/>
        <xsl:text>{</xsl:text>
        <xsl:text> &quot;</xsl:text><xsl:value-of select="@name" /><xsl:text>&quot;</xsl:text>
        <xsl:text>, </xsl:text>
        <xsl:call-template name="get_eType">
          <xsl:with-param name="A"><xsl:value-of select="@type"/></xsl:with-param>
        </xsl:call-template>
        <xsl:text>, </xsl:text>
        <xsl:call-template name="get_type_uuid_ptr">
          <xsl:with-param name="A"><xsl:value-of select="@type"/></xsl:with-param>
        </xsl:call-template>
        <xsl:text>, &quot;</xsl:text><xsl:value-of select="@typec"/><xsl:text>&quot;</xsl:text>
        <xsl:text> }</xsl:text>
        <xsl:if test="following-sibling::*[not(@deprecated='yes')]/@name">, </xsl:if>
      </xsl:for-each>
      <xsl:call-template name="newline"/>
      <xsl:text>}; </xsl:text>
      <xsl:call-template name="newline"/>
    </xsl:if>

		<!-- FUNCTION -->
		<xsl:text>static const ni::sMethodDef </xsl:text><xsl:value-of select="$A"/><xsl:text>_</xsl:text><xsl:value-of select="@name"/><xsl:text> = {</xsl:text>

		<xsl:call-template name="newline"/>
		<xsl:call-template name="tab"/>
		<xsl:text>&quot;</xsl:text>
			<xsl:if test="@automationname"><xsl:value-of select="@automationname"/></xsl:if>
			<xsl:if test="not(@automationname)"><xsl:value-of select="@name"/></xsl:if>
		<xsl:text>&quot;,</xsl:text>

		<xsl:call-template name="newline"/>
    <xsl:call-template name="tab"/>

		<xsl:choose>
			<xsl:when test="@property='set'">
				<xsl:text>ni::eTypeFlags_MethodSetter|</xsl:text>
			</xsl:when>
			<xsl:when test="@property='get'">
				<xsl:text>ni::eTypeFlags_MethodGetter|</xsl:text>
			</xsl:when>
		</xsl:choose>

		<xsl:if test="@optional='yes'">
			<xsl:text>ni::eTypeFlags_MethodOptional|</xsl:text>
		</xsl:if>
		<xsl:text>0|</xsl:text>

    <xsl:choose>
      <xsl:when test="(@property='set') and (count(&PARAMETERS;)=1)">
        <xsl:call-template name="get_eType">
          <xsl:with-param name="A"><xsl:value-of select="parameter[1]/@type"/></xsl:with-param>
          <xsl:with-param name="B"><xsl:value-of select="@noautomation"/></xsl:with-param>
        </xsl:call-template>
        <xsl:text>, </xsl:text>
        <xsl:call-template name="get_type_uuid_ptr">
          <xsl:with-param name="A"><xsl:value-of select="parameter[1]/@type"/></xsl:with-param>
        </xsl:call-template>
      </xsl:when>
      <xsl:otherwise>
        <xsl:call-template name="get_eType">
          <xsl:with-param name="A"><xsl:value-of select="return/@type"/></xsl:with-param>
          <xsl:with-param name="B"><xsl:value-of select="@noautomation"/></xsl:with-param>
        </xsl:call-template>
        <xsl:text>, </xsl:text>
        <xsl:call-template name="get_type_uuid_ptr">
          <xsl:with-param name="A"><xsl:value-of select="return/@type"/></xsl:with-param>
        </xsl:call-template>
      </xsl:otherwise>
    </xsl:choose>

    <xsl:text>, &quot;</xsl:text><xsl:value-of select="return/@typec"/><xsl:text>&quot;,</xsl:text>

		<xsl:call-template name="newline"/>
    <xsl:call-template name="tab"/>
    <xsl:value-of select="count(&PARAMETERS;)"/><xsl:text>, </xsl:text>
		<xsl:if test="not(count(&PARAMETERS;)=0)">
      <xsl:value-of select="$A"/><xsl:text>_</xsl:text><xsl:value-of select="@name"/><xsl:text>_Parameters,</xsl:text>
    </xsl:if>
		<xsl:if test="count(&PARAMETERS;)=0">
      <xsl:text>NULL,</xsl:text>
    </xsl:if>

		<xsl:call-template name="newline"/>

		<xsl:choose>
			<xsl:when test="not(@noautomation='yes')">
				<xsl:text>#ifndef niConfig_NoXCALL
  XCALL_CIMPL(</xsl:text><xsl:value-of select="$A"/><xsl:text>_</xsl:text><xsl:value-of select="@name"/><xsl:text>)
#else
  NULL
#endif</xsl:text>
			</xsl:when>
			<xsl:otherwise>
				<xsl:text>NULL</xsl:text>
			</xsl:otherwise>
		</xsl:choose>
		<xsl:call-template name="newline"/>

		<xsl:text>};</xsl:text>
		<xsl:call-template name="newline"/>

	</xsl:template>

<!-- ************************************************************ -->
	<xsl:template name="get_min_features_begin">
        <xsl:if test="@min_features > 0">
            <xsl:text>#if niMinFeatures(</xsl:text>
            <xsl:value-of select="@min_features" />
            <xsl:text>)</xsl:text>
            <xsl:call-template name="newline" />
        </xsl:if>
    </xsl:template>

<!-- ************************************************************ -->
	<xsl:template name="get_min_features_end">
        <xsl:if test="@min_features > 0">
            <xsl:text>#endif // niMinFeatures(</xsl:text>
            <xsl:value-of select="@min_features" />
            <xsl:text>)</xsl:text>
            <xsl:call-template name="newline" />
        </xsl:if>
    </xsl:template>

<!-- ************************************************************ -->
	<xsl:template name="get_var_retvalue">
		<xsl:param name="A" />

		<!-- basic type -->
		<xsl:choose>
			<!-- special cases -->
			<xsl:when test="contains($A,'**')">
				<xsl:message terminate='yes'>E/ !!!! (get_var_retvalue) Dispacth wrapper cannot return pointer of pointer.</xsl:message>
			</xsl:when>

			<xsl:when test="contains($A,'const') and contains($A,'achar') and contains($A,'*')">
				<xsl:text>_var_.GetACharConstPointer()</xsl:text>
			</xsl:when>
			<xsl:when test="contains($A,'const') and contains($A,'cchar') and contains($A,'*')">
				<xsl:message terminate='yes'>E/ !!!! (cchar) Char type has been deprecated, use achar instead.</xsl:message>
			</xsl:when>
			<xsl:when test="contains($A,'const') and contains($A,' char') and contains($A,'*')">
				<xsl:message terminate='yes'>E/ !!!! (char) Char type has been deprecated, use achar instead.</xsl:message>
            </xsl:when>
			<xsl:when test="contains($A,'const') and contains($A,'uchar') and contains($A,'*')">
				<xsl:message terminate='yes'>E/ !!!! (uchar) Char type has been deprecated, use achar instead.</xsl:message>
            </xsl:when>
			<xsl:when test="contains($A,'const') and contains($A,'gchar') and contains($A,'*')">
				<xsl:message terminate='yes'>E/ !!!! (gchar) Char type has been deprecated, use achar instead.</xsl:message>
            </xsl:when>
			<xsl:when test="contains($A,'const') and contains($A,'xchar') and contains($A,'*')">
				<xsl:message terminate='yes'>E/ !!!! (xchar) Char type has been deprecated, use achar instead.</xsl:message>
            </xsl:when>
			<xsl:when test="&AISCOLLECTION;">
				<xsl:if test="not(&AISPTR;)"><xsl:message terminate='yes'>E/ !!!! (get_var_retvalue) Dispacth wrapper cannot return collection by value.</xsl:message></xsl:if>

        <xsl:if test="contains($A,'Ptr&lt;')">
          <xsl:text>(</xsl:text><xsl:value-of select="substring-before(substring-after($A,'&lt;'),'&gt;')"/><xsl:text>*)_var_.GetRawIUnknownPointerAndSetNull()</xsl:text>
        </xsl:if>
        <xsl:if test="not(contains($A,'Ptr&lt;'))">
          <xsl:text>(</xsl:text><xsl:value-of select="$A"/><xsl:text>)_var_.GetRawIUnknownPointerAndSetNull()</xsl:text>
        </xsl:if>
			</xsl:when>
			<xsl:when test="&AISINTERFACE;">
				<xsl:if test="not(&AISPTR;)"><xsl:message terminate="yes">E/ !!!! (get_var_retvalue) Dispacth wrapper cannot return interface by value. (<xsl:value-of select="$A"/>)</xsl:message></xsl:if>

        <xsl:if test="contains($A,'Ptr&lt;')">
          <xsl:text>(</xsl:text><xsl:value-of select="substring-before(substring-after($A,'&lt;'),'&gt;')"/><xsl:text>*)_var_.GetRawIUnknownPointerAndSetNull()</xsl:text>
        </xsl:if>
        <xsl:if test="not(contains($A,'Ptr&lt;'))">
          <xsl:text>(</xsl:text><xsl:value-of select="$A"/><xsl:text>)_var_.GetRawIUnknownPointerAndSetNull()</xsl:text>
        </xsl:if>
			</xsl:when>
			<xsl:when test="(&AISPTR; and not(contains($A,'&amp;'))) or contains($A,'tPtr')">
				<xsl:message terminate='yes'>E/ !!!! (get_var_retvalue) Dispacth wrapper cannot return pointer of anything else than interfaces.</xsl:message>
			</xsl:when>
			<!-- general cases -->
			<xsl:otherwise>
				<xsl:if test="contains($A,'&amp;')">
					<xsl:text>__INVALID_RET_REF__(</xsl:text>
					<xsl:message terminate='yes'>E/ !!!! (get_var_retvalue) Can't use a value type returned by reference.</xsl:message>
				</xsl:if>
				<xsl:choose>
					<xsl:when test="&AISFLAGS;"><xsl:text>(</xsl:text><xsl:value-of select="$A"/><xsl:text>)_var_.GetU32()</xsl:text></xsl:when>
					<xsl:when test="contains($A,'sRecti')"><xsl:text>_var_.GetVec4i()</xsl:text></xsl:when>
					<xsl:when test="contains($A,'sRectf')"><xsl:text>_var_.GetVec4f()</xsl:text></xsl:when>
					<xsl:when test="&MATCH_VAR;"><xsl:text>_var_</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tFVF')"><xsl:text>_var_.GetU32()</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tType')"><xsl:text>_var_.GetU32()</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tTime')"><xsl:text>_var_.GetI64()</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tVersion')"><xsl:text>_var_.GetU32()</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tEnum')"><xsl:text>_var_.GetU32()</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tHandle')"><xsl:text>_var_.GetU32()</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tOffset')"><xsl:text>_var_.GetU32()</xsl:text></xsl:when>
					<xsl:when test="&AISENUM;"><xsl:text>(</xsl:text><xsl:value-of select="$A"/><xsl:text>)_var_.GetU32()</xsl:text></xsl:when>
					<xsl:when test="contains($A,'cString')"><xsl:text>_var_.GetString()</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tBool')"><xsl:text>(ni::tBool)_var_.GetI8()</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tSize')"><xsl:text>_var_.GetU32()</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tI8')"><xsl:text>_var_.GetI8()</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tU8')"><xsl:text>_var_.GetU8()</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tI16')"><xsl:text>_var_.GetI16()</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tU16')"><xsl:text>_var_.GetU16()</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tI32')"><xsl:text>_var_.GetI32()</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tU32')"><xsl:text>_var_.GetU32()</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tI64')"><xsl:text>_var_.GetI64()</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tU64')"><xsl:text>_var_.GetU64()</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tUUID')"><xsl:text>_var_.GetUUID()</xsl:text></xsl:when>
					<xsl:when test="contains($A,'sUUID')"><xsl:text>_var_.GetUUID()</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tInt')"><xsl:text>_var_.GetInt()</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tUInt')"><xsl:text>_var_.GetUInt()</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tF32')"><xsl:text>_var_.GetF32()</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tFloat')"><xsl:text>_var_.GetF32()</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tF64')"><xsl:text>_var_.GetF64()</xsl:text></xsl:when>
					<xsl:when test="contains($A,'sVec2i')"><xsl:text>_var_.GetVec2i()</xsl:text></xsl:when>
					<xsl:when test="contains($A,'sVec3i')"><xsl:text>_var_.GetVec3i()</xsl:text></xsl:when>
					<xsl:when test="contains($A,'sVec4i')"><xsl:text>_var_.GetVec4i()</xsl:text></xsl:when>
					<xsl:when test="contains($A,'sVec2f')"><xsl:text>_var_.GetVec2f()</xsl:text></xsl:when>
					<xsl:when test="contains($A,'sVec3f')"><xsl:text>_var_.GetVec3f()</xsl:text></xsl:when>
					<xsl:when test="contains($A,'sVec4f')"><xsl:text>_var_.GetVec4f()</xsl:text></xsl:when>
					<xsl:when test="contains($A,'sPlanef')"><xsl:text>_var_.GetVec4f()</xsl:text></xsl:when>
					<xsl:when test="contains($A,'sColor3ub')"><xsl:text>_var_.GetU32()</xsl:text></xsl:when>
					<xsl:when test="contains($A,'sColor4ub')"><xsl:text>_var_.GetU32()</xsl:text></xsl:when>
					<xsl:when test="contains($A,'sColor3f')"><xsl:text>_var_.GetVec3f()</xsl:text></xsl:when>
					<xsl:when test="contains($A,'sColor4f')"><xsl:text>_var_.GetVec4f()</xsl:text></xsl:when>
					<xsl:when test="contains($A,'sQuatf')"><xsl:text>_var_.GetVec4f()</xsl:text></xsl:when>
					<xsl:when test="contains($A,'sMatrixf')"><xsl:text>_var_.GetMatrixf()</xsl:text></xsl:when>
					<xsl:otherwise>
						<xsl:message terminate='yes'>E/ !!!! (get_var_retvalue) Dispatch wrapper invalid return type: <xsl:value-of select="$A"/></xsl:message>
						<xsl:text>Ptr</xsl:text>
					</xsl:otherwise>
				</xsl:choose>
				<xsl:if test="contains($A,'&amp;')">
					<xsl:text>)</xsl:text>
				</xsl:if>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

<!-- ************************************************************ -->
	<xsl:template name="get_dispatch_method_wrapper">
		<xsl:call-template name="newline"/>
		<xsl:call-template name="tab"/>
		<xsl:text>// Method (</xsl:text><xsl:value-of select="position()-1"/><xsl:text>): </xsl:text><xsl:value-of select="../@name"/><xsl:text>::</xsl:text><xsl:value-of select="@name"/>
		<xsl:call-template name="newline"/>

		<xsl:call-template name="tab"/>
		<xsl:value-of select="return/@type"/>
		<xsl:text> __stdcall </xsl:text>
		<xsl:value-of select="@name"/>
		<xsl:text>(</xsl:text>
			<xsl:for-each select="&PARAMETERS;">
				<xsl:value-of select="@type"/>
				<xsl:text> </xsl:text>
				<xsl:value-of select="@name"/>
				<xsl:if test="following-sibling::*[not(@deprecated='yes')]/@name">, </xsl:if>
			</xsl:for-each>
		<xsl:text>)</xsl:text><xsl:if test="@const='yes'"><xsl:text> const</xsl:text></xsl:if><xsl:text> {</xsl:text>

		<xsl:if test="not(return/@type='void')">
      <xsl:call-template name="newline"/>
      <xsl:call-template name="tab"/>
      <xsl:call-template name="tab"/>
      <xsl:text>ni::Var _var_;</xsl:text>
    </xsl:if>
		<xsl:if test="not(count(&PARAMETERS;)=0)">
      <xsl:call-template name="newline"/>
      <xsl:call-template name="tab"/>
      <xsl:call-template name="tab"/>
      <xsl:text>ni::Var _params_[</xsl:text><xsl:value-of select="count(&PARAMETERS;)"/><xsl:text>];</xsl:text>
      <xsl:for-each select="&PARAMETERS;">
        <xsl:call-template name="newline"/>
        <xsl:call-template name="tab"/>
        <xsl:call-template name="tab"/>
        <xsl:text>_params_[</xsl:text><xsl:value-of select="position()-1"/><xsl:text>] = </xsl:text>
        <xsl:if test="(starts-with(@type,'e') or starts-with(@type,'const e') or contains(@type,'::e'))">
          <xsl:text>(ni::tU32)</xsl:text>
        </xsl:if>
        <xsl:if test="(starts-with(@type,'i') or starts-with(@type,'const i') or contains(@type,'::i')) and not(starts-with(@type,'int'))">
          <xsl:if test="starts-with(@type,'const')">
            <xsl:text>niIUnknownCCast(</xsl:text>
          </xsl:if>
          <xsl:if test="not(starts-with(@type,'const'))">
            <xsl:text>niIUnknownCast(</xsl:text>
          </xsl:if>
        </xsl:if>
        <xsl:value-of select="@name"/>
        <xsl:if test="(starts-with(@type,'i') or starts-with(@type,'const i') or contains(@type,'::i')) and not(starts-with(@type,'int'))">
          <xsl:text>)</xsl:text>
        </xsl:if>
        <xsl:text>;</xsl:text>
      </xsl:for-each>
		</xsl:if>

		<xsl:call-template name="newline"/>
		<xsl:call-template name="tab"/>
		<xsl:call-template name="tab"/>

		<xsl:if test="not(return/@type='void')">
      <xsl:text>if (</xsl:text>
    </xsl:if>

    <xsl:text>((ni::iDispatch*)(mprotected_pAggregateParent))-&gt;CallMethod(&amp;</xsl:text><xsl:value-of select="../@name"/><xsl:text>_</xsl:text><xsl:value-of select="@name"/><xsl:text>,</xsl:text><xsl:value-of select="position()-1"/><xsl:text>,</xsl:text>
		<xsl:choose>
			<xsl:when test="not(count(&PARAMETERS;)=0)"><xsl:text>_params_,</xsl:text></xsl:when>
			<xsl:otherwise><xsl:text>NULL,</xsl:text></xsl:otherwise>
		</xsl:choose>
		<xsl:value-of select="count(&PARAMETERS;)"/><xsl:text>,</xsl:text>
		<xsl:choose>
			<xsl:when test="not(return/@type='void')"><xsl:text>&amp;_var_</xsl:text></xsl:when>
			<xsl:otherwise><xsl:text>NULL</xsl:text></xsl:otherwise>
		</xsl:choose>
    <xsl:text>)</xsl:text>

		<xsl:choose>
		<xsl:when test="contains(return/@typei,'_Variant')"><xsl:text>) {
  return </xsl:text><xsl:call-template name="get_var_retvalue"><xsl:with-param name="A"><xsl:value-of select="return/@type"/></xsl:with-param></xsl:call-template><xsl:text>;
  }</xsl:text></xsl:when>
		<xsl:when test="not(return/@type='void')"><xsl:text>) {
      if (VarIsType(&amp;_var_,</xsl:text><xsl:value-of select="../@name"/><xsl:text>_</xsl:text><xsl:value-of select="@name"/><xsl:text>.mReturnType,ni::eFalse)) {
        return </xsl:text><xsl:call-template name="get_var_retvalue"><xsl:with-param name="A"><xsl:value-of select="return/@type"/></xsl:with-param></xsl:call-template><xsl:text>;
      }
    }</xsl:text></xsl:when>
		<xsl:otherwise><xsl:text>;</xsl:text>
		</xsl:otherwise>
		</xsl:choose>

		<xsl:if test="not(return/@type='void')">
      <xsl:call-template name="newline"/>
      <xsl:call-template name="tab"/>
      <xsl:call-template name="tab"/>
      <xsl:call-template name="get_zero_retvalue">
        <xsl:with-param name="A">
          <xsl:value-of select="return/@type"/>
        </xsl:with-param>
      </xsl:call-template>
    </xsl:if>

<xsl:text>
	}
</xsl:text>

	</xsl:template>

<!-- ************************************************************ -->
	<xsl:template name="get_dispatch_wrapper">
<xsl:text>struct niHidden DispatchWrapper_</xsl:text><xsl:value-of select="@name"/><xsl:text> : public ni::cIUnknownImpl&lt;</xsl:text><xsl:value-of select="../@name"/><xsl:text>::</xsl:text><xsl:value-of select="@name"/><xsl:text>,ni::eIUnknownImplFlags_Aggregatable|ni::eIUnknownImplFlags_NoRefCount&gt;
{
	DispatchWrapper_</xsl:text><xsl:value-of select="@name"/><xsl:text>(ni::iDispatch* apDispatch) {
    mprotected_pAggregateParent = apDispatch;
	}
	~DispatchWrapper_</xsl:text><xsl:value-of select="@name"/><xsl:text>() {
    this-&gt;Invalidate();
  }

	ni::iUnknown* __stdcall QueryInterface(const ni::tUUID&amp; aIID) niImpl {
    if (aIID == niGetInterfaceUUID(</xsl:text><xsl:value-of select="@name"/><xsl:text>))
      return this;
		return mprotected_pAggregateParent-&gt;QueryInterface(aIID);
	}
	void __stdcall ListInterfaces(ni::iMutableCollection* apLst, ni::tU32 anFlags) const niImpl {
    apLst->Add(niGetInterfaceUUID(</xsl:text><xsl:value-of select="@name"/><xsl:text>));
		mprotected_pAggregateParent-&gt;ListInterfaces(apLst,anFlags);
	}
</xsl:text>
	<xsl:for-each select="&METHODS;">
		<xsl:call-template name="get_dispatch_method_wrapper" />
	</xsl:for-each>
<xsl:text>
	static ni::iUnknown* Create(ni::iDispatch* apDispatch) {
		niAssert(niIsOK(apDispatch));
    if (!apDispatch-&gt;InitializeMethods(Methods_</xsl:text><xsl:value-of select='@name'/><xsl:text>, niCountOf(Methods_</xsl:text><xsl:value-of select='@name'/><xsl:text>))) {
      niError(niFmt("Can't initialize dispatch methods for %s.",  "</xsl:text><xsl:value-of select='@name'/><xsl:text>"));
      return NULL;
    }
		return niNew DispatchWrapper_</xsl:text><xsl:value-of select="@name"/><xsl:text>(apDispatch);
	}
};
</xsl:text>
	</xsl:template>

<!-- ************************************************************ -->
	<xsl:template name="get_type_uuid_ptr">
		<xsl:param name="A" />
		<xsl:choose>
			<xsl:when test="not($A)">
				<xsl:text>NULL</xsl:text>
      </xsl:when>
			<xsl:when test="&AISENUM;">
				<xsl:text>NULL</xsl:text>
			</xsl:when>
			<xsl:when test="&AISFLAGS;">
				<xsl:text>NULL</xsl:text>
			</xsl:when>
			<xsl:when test="&AISSINKLIST;">
        <xsl:text>&amp;niGetInterfaceUUID(ni::iSinkList)</xsl:text>
      </xsl:when>
			<xsl:when test="&AISCOLLECTION;">
        <xsl:text>&amp;niGetInterfaceUUID(ni::iCollection)</xsl:text>
      </xsl:when>
			<xsl:when test="&AISINTERFACE;">
				<xsl:choose>
					<xsl:when test="contains($A,'Ptr&lt;')">
						<xsl:text>&amp;niGetInterfaceUUID(</xsl:text>
						<xsl:value-of select="substring-after(substring-before($A,'&gt;'),'&lt;')"/>
						<xsl:text>)</xsl:text>
					</xsl:when>
					<xsl:when test="starts-with($A,'const ')">
						<xsl:text>&amp;niGetInterfaceUUID(</xsl:text>
						<xsl:value-of select="normalize-space(substring($A,7,string-length(substring-before($A,' *'))-5))"/>
						<xsl:text>)</xsl:text>
					</xsl:when>
					<xsl:otherwise>
						<xsl:text>&amp;niGetInterfaceUUID(</xsl:text>
						<xsl:value-of select="normalize-space(substring($A,1,string-length(substring-before($A,' *'))))"/>
						<xsl:text>)</xsl:text>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:when>
			<xsl:otherwise>
				<xsl:text>NULL</xsl:text>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

<!-- ************************************************************ -->
	<xsl:template name="get_eType">
		<xsl:param name="A" />
		<xsl:param name="B" />
		<xsl:choose>
			<xsl:when test="starts-with($A,'const')"><xsl:text>ni::eTypeFlags_Constant|</xsl:text></xsl:when>
			<xsl:otherwise><xsl:text></xsl:text></xsl:otherwise>
		</xsl:choose>

		<!-- basic type -->
		<xsl:text>ni::eType_</xsl:text>
		<xsl:choose>
			<!-- special cases -->
			<xsl:when test="contains($A,'**')">
				<xsl:message>W/ !!!! Pointer of pointer not allowed.</xsl:message>
				<xsl:text>Ptr, NULL, &amp;ni::kuuidZero</xsl:text>
			</xsl:when>
			<xsl:when test="&AISCOLLECTION;">
				<xsl:if test="not(&AISPTR;)"><xsl:message terminate="yes">E/ !!!! Collections cannot be passed by value. (<xsl:value-of select="$A"/>)</xsl:message></xsl:if>
				<xsl:text>IUnknown|ni::eTypeFlags_Pointer</xsl:text>
			</xsl:when>
			<xsl:when test="&AISINTERFACE;">
				<xsl:if test="not(&AISPTR;)"><xsl:message terminate="yes">E/ !!!! Interfaces cannot be passed by value. (<xsl:value-of select="../../@name"/>::<xsl:value-of select="../@name"/>::<xsl:value-of select="$A"/>)</xsl:message></xsl:if>
				<xsl:text>IUnknown|ni::eTypeFlags_Pointer</xsl:text>
			</xsl:when>
			<xsl:when test="&AISENUM;">
				<xsl:text>Enum</xsl:text>
			</xsl:when>
			<xsl:when test="&AISFLAGS;">
				<xsl:text>Enum</xsl:text>
			</xsl:when>
			<!-- general cases -->
			<xsl:otherwise>
				<xsl:choose>
					<xsl:when test="starts-with($A,'void')"><xsl:text>Null</xsl:text></xsl:when>
					<xsl:when test="contains($A,'sRecti')"><xsl:text>Vec4i</xsl:text></xsl:when>
					<xsl:when test="contains($A,'sRectf')"><xsl:text>Vec4f</xsl:text></xsl:when>
					<xsl:when test="&MATCH_VAR;">
						<xsl:text>Variant</xsl:text>
					</xsl:when>
					<xsl:when test="contains($A,'tFVF')"><xsl:text>U32</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tType')"><xsl:text>U32</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tTime')"><xsl:text>I64</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tVersion')"><xsl:text>U32</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tEnum')"><xsl:text>U32</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tHandle')"><xsl:text>UIntPtr</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tIntPtr')"><xsl:text>IntPtr</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tUIntPtr')"><xsl:text>UIntPtr</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tPtr')"><xsl:text>Ptr</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tOffset')"><xsl:text>Offset</xsl:text></xsl:when>
					<xsl:when test="contains($A,'cCString')"><xsl:text>CString</xsl:text></xsl:when>
					<xsl:when test="contains($A,'cGString')"><xsl:text>GString</xsl:text></xsl:when>
					<xsl:when test="contains($A,'cXString')"><xsl:text>XString</xsl:text></xsl:when>
					<xsl:when test="contains($A,'cUString')"><xsl:text>UString</xsl:text></xsl:when>
					<xsl:when test="contains($A,'cString')"><xsl:text>String</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tBool')"><xsl:text>I8</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tSize')"><xsl:text>Size</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tI8')"><xsl:text>I8</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tU8')"><xsl:text>U8</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tI16')"><xsl:text>I16</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tU16')"><xsl:text>U16</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tI32')"><xsl:text>I32</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tU32')"><xsl:text>U32</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tI64')"><xsl:text>I64</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tU64')"><xsl:text>U64</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tUUID')"><xsl:text>UUID</xsl:text></xsl:when>
					<xsl:when test="contains($A,'sUUID')"><xsl:text>UUID</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tInt')"><xsl:text>Int</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tUInt')"><xsl:text>UInt</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tF32')"><xsl:text>F32</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tFloat')"><xsl:text>F32</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tF64')"><xsl:text>F64</xsl:text></xsl:when>
					<xsl:when test="contains($A,'sVec2i')"><xsl:text>Vec2i</xsl:text></xsl:when>
					<xsl:when test="contains($A,'sVec3i')"><xsl:text>Vec3i</xsl:text></xsl:when>
					<xsl:when test="contains($A,'sVec4i')"><xsl:text>Vec4i</xsl:text></xsl:when>
					<xsl:when test="contains($A,'sVec2f')"><xsl:text>Vec2f</xsl:text></xsl:when>
					<xsl:when test="contains($A,'sVec3f')"><xsl:text>Vec3f</xsl:text></xsl:when>
					<xsl:when test="contains($A,'sVec4f')"><xsl:text>Vec4f</xsl:text></xsl:when>
					<xsl:when test="contains($A,'sPlanef')"><xsl:text>Vec4f</xsl:text></xsl:when>
					<xsl:when test="contains($A,'sColor3ub')"><xsl:text>U32</xsl:text></xsl:when>
					<xsl:when test="contains($A,'sColor4ub')"><xsl:text>U32</xsl:text></xsl:when>
					<xsl:when test="contains($A,'sColor3f')"><xsl:text>Vec3f</xsl:text></xsl:when>
					<xsl:when test="contains($A,'sColor4f')"><xsl:text>Vec4f</xsl:text></xsl:when>
					<xsl:when test="contains($A,'sQuatf')"><xsl:text>Vec4f</xsl:text></xsl:when>
					<xsl:when test="contains($A,'sMatrixf')"><xsl:text>Matrixf</xsl:text></xsl:when>
					<xsl:when test="contains($A,'achar') or contains($A,'cchar') or contains($A,' char') or contains($A,'uchar') or contains($A,'gchar') or contains($A,'xchar')">
						<xsl:if test="not(starts-with($A,'const') and &AISPTR;)">
							<xsl:message terminate='yes'>E/ Char types can only be constant pointers.</xsl:message>
						</xsl:if>
						<xsl:choose>
							<xsl:when test="contains($A,'achar')"><xsl:text>AChar</xsl:text></xsl:when>
              <xsl:otherwise>
                <xsl:message terminate='yes'>E/ Char type has been deprecated, use achar instead.</xsl:message>
              </xsl:otherwise>
						</xsl:choose>
					</xsl:when>
					<xsl:otherwise>
						<xsl:choose>
							<xsl:when test="$B='yes'">
								<xsl:text>Null</xsl:text>
							</xsl:when>
<!-- 							<xsl:when test="/niIDL/@unknowntype"> -->
<!-- 								<xsl:value-of select="/niIDL/@unknowntype" /> -->
<!-- 							</xsl:when> -->
							<xsl:otherwise>
								<xsl:message terminate="no">W/ Unknown type '<xsl:value-of select="$A"/>'</xsl:message>
								<xsl:text>Null</xsl:text>
							</xsl:otherwise>
						</xsl:choose>
					</xsl:otherwise>
				</xsl:choose>
				<xsl:if test="&AISPTR;"><xsl:text>|ni::eTypeFlags_Pointer</xsl:text></xsl:if>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

</xsl:stylesheet>
