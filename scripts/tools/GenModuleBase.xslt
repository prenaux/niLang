<?xml version="1.0" encoding="UTF-8" ?>
<!-- (c) 2022 The niLang Authors -->

<!DOCTYPE stylesheet [
	<!ENTITY AISFLAGS "((starts-with($A,'t') or starts-with($A,'const t')) and contains($A,'Flags'))">
	<!ENTITY AISENUM "(starts-with($A,'e') or starts-with($A,'const e') or contains($A,'::e'))">
	<!ENTITY MATCH_VAR "starts-with($A,'Var') or starts-with($A,'const Var') or starts-with($A,'ni::Var') or starts-with($A,'const ni::Var')">
]>

<xsl:stylesheet version="1.0"
				xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
				xmlns:math="http://exslt.org/math">

<!-- ************************************************************ -->
	<xsl:template name="newline">
		<xsl:text>&#10;</xsl:text>
	</xsl:template>
<!-- ************************************************************ -->
	<xsl:template name="tab">
		<xsl:text>  </xsl:text>
	</xsl:template>

<!-- ************************************************************ -->
    <xsl:template name="substring-after-last">
        <xsl:param name="string" />
        <xsl:param name="delimiter" />
        <xsl:choose>
            <xsl:when test="contains($string, $delimiter)">
                <xsl:call-template name="substring-after-last">
                    <xsl:with-param name="string"
                                    select="substring-after($string, $delimiter)" />
                    <xsl:with-param name="delimiter" select="$delimiter" />
                </xsl:call-template>
            </xsl:when>
            <xsl:otherwise><xsl:value-of select="$string" /></xsl:otherwise>
        </xsl:choose>
    </xsl:template>

<!-- ************************************************************
* Get a type name without it's namespace
************************************************************* -->
	<xsl:template name="get_without_namespace">
		<xsl:param name="A" />
		<xsl:choose>
			<xsl:when test="contains($A,'::')">
				<xsl:value-of select="substring-after($A,'::')"/>
			</xsl:when>
			<xsl:otherwise>
				<xsl:value-of select="$A"/>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>
<!-- ************************************************************
* Get the namespace of a typename
************************************************************* -->
	<xsl:template name="get_namespace">
		<xsl:param name="A" />
		<xsl:if test="contains($A,'::')">
			<xsl:value-of select="substring-before($A,'::')"/>
		</xsl:if>
	</xsl:template>
<!-- ************************************************************
* Get the namespace of a typename, with the appropriate :: if
* it's a namespace
************************************************************* -->
	<xsl:template name="get_namespace_and_colons">
		<xsl:param name="A" />
		<xsl:if test="contains($A,'::')">
			<xsl:value-of select="substring-before($A,'::')"/><xsl:text>::</xsl:text>
		</xsl:if>
	</xsl:template>

<!-- ************************************************************ -->
	<xsl:template name="get_zero_retvalue">
		<xsl:param name="A" />

		<!-- basic type -->
		<xsl:choose>
			<!-- Void -->
			<xsl:when test="$A='void'">
				<xsl:text>return;</xsl:text>
			</xsl:when>
			<!-- Pointers -->
			<xsl:when test="contains($A,'*') and contains($A,'&amp;')">
				<xsl:message terminate='yes'>!!!! (get_zero_retvalue) Cannot return reference of pointer '<xsl:value-of select="$A"/>' in '<xsl:value-of select="../@name"/>::<xsl:value-of select="@name"/>'.</xsl:message>
			</xsl:when>
			<xsl:when test="contains($A,'*')">
				<xsl:choose>
				<xsl:when test="contains($A,'const') and contains($A,'achar')">
					<xsl:text>return AZEROSTR;</xsl:text>
				</xsl:when>
				<xsl:otherwise>
					<xsl:text>return NULL;</xsl:text>
				</xsl:otherwise>
				</xsl:choose>
			</xsl:when>
			<!-- References -->
			<xsl:when test="contains($A,'&amp;')">
				<!--
				<xsl:if test="not(contains($A,'const'))">
					<xsl:message terminate='yes'>!!!! (get_zero_retvalue) Null implementation cannot return non-constant reference of '<xsl:value-of select="$A"/>' in '<xsl:value-of select="../@name"/>::<xsl:value-of select="@name"/>' by value.</xsl:message>
				</xsl:if>
				-->
				<xsl:choose>
				<xsl:when test="&AISFLAGS; or &AISENUM;"><xsl:text>static const ni::tU32 _ret = 0; return (</xsl:text><xsl:value-of select="$A"/><xsl:text>)_ret;</xsl:text></xsl:when>
				<xsl:when test="contains($A,'tTime') and not(contains($A,'Event'))"><xsl:text>static const ni::tTime _ret = (ni::tTime)0; return _ret;</xsl:text></xsl:when>
				<xsl:when test="contains($A,'sRecti')">
                    <xsl:text>return ni::sRecti::Null();</xsl:text>
                </xsl:when>
				<xsl:when test="contains($A,'sRectf')">
                    <xsl:text>return ni::sRectf::Null();</xsl:text>
                </xsl:when>
				<xsl:when test="starts-with($A,'Var') or starts-with($A,'const Var')"><xsl:text>static ni::Var _ret = niVarNull; return _ret;</xsl:text></xsl:when>
				<xsl:when test="contains($A,'tFVF')"><xsl:text>static const ni::tFVF _ret = 0; return _ret;</xsl:text></xsl:when>
				<xsl:when test="contains($A,'tType')"><xsl:text>static const ni::tType _ret = 0; return _ret;</xsl:text></xsl:when>
				<xsl:when test="contains($A,'tVersion')"><xsl:text>static const ni::tVersion _ret = 0; return _ret;</xsl:text></xsl:when>
				<xsl:when test="contains($A,'tEnum')"><xsl:text>static const ni::tEnum _ret = 0; return _ret;</xsl:text></xsl:when>
				<xsl:when test="contains($A,'tHandle')"><xsl:text>static const ni::tHandle _ret = 0; return _ret;</xsl:text></xsl:when>
				<xsl:when test="contains($A,'tOffset')"><xsl:text>static const ni::tOffset _ret = 0; return _ret;</xsl:text></xsl:when>
				<xsl:when test="contains($A,'cString')"><xsl:text>static const ni::cString _ret = ni::cString(); return _ret;</xsl:text></xsl:when>
				<xsl:when test="contains($A,'tBool')"><xsl:text>static const ni::tBool _ret = ni::eFalse; return _ret;</xsl:text></xsl:when>
				<xsl:when test="contains($A,'tSize')"><xsl:text>static const ni::tSize _ret = 0; return _ret;</xsl:text></xsl:when>
				<xsl:when test="contains($A,'tI8')"><xsl:text>static const ni::tI8 _ret = 0; return _ret;</xsl:text></xsl:when>
				<xsl:when test="contains($A,'tU8')"><xsl:text>static const ni::tU8 _ret = 0; return _ret;</xsl:text></xsl:when>
				<xsl:when test="contains($A,'tI16')"><xsl:text>static const ni::tI16 _ret = 0; return _ret;</xsl:text></xsl:when>
				<xsl:when test="contains($A,'tU16')"><xsl:text>static const ni::tU16 _ret = 0; return _ret;</xsl:text></xsl:when>
				<xsl:when test="contains($A,'tI32')"><xsl:text>static const ni::tI32 _ret = 0; return _ret;</xsl:text></xsl:when>
				<xsl:when test="contains($A,'tU32')"><xsl:text>static const ni::tU32 _ret = 0; return _ret;</xsl:text></xsl:when>
				<xsl:when test="contains($A,'tI64')"><xsl:text>static const ni::tI64 _ret = 0; return _ret;</xsl:text></xsl:when>
				<xsl:when test="contains($A,'tU64')"><xsl:text>static const ni::tU64 _ret = 0; return _ret;</xsl:text></xsl:when>
				<xsl:when test="contains($A,'tUUID')"><xsl:text>static const ni::tUUID _ret = kUUIDZero; return _ret;</xsl:text></xsl:when>
				<xsl:when test="contains($A,'tInt')"><xsl:text>static const ni::tInt _ret = 0; return _ret;</xsl:text></xsl:when>
				<xsl:when test="contains($A,'tUInt')"><xsl:text>static const ni::tUInt _ret = 0; return _ret;</xsl:text></xsl:when>
				<xsl:when test="contains($A,'tF32')"><xsl:text>static const ni::tF32 _ret = 0; return _ret;</xsl:text></xsl:when>
				<xsl:when test="contains($A,'tFloat')"><xsl:text>static const ni::tF32 _ret = 0; return _ret;</xsl:text></xsl:when>
				<xsl:when test="contains($A,'tF64')"><xsl:text>static const ni::tF64 _ret = 0; return _ret;</xsl:text></xsl:when>
				<xsl:when test="contains($A,'sPlanef')"><xsl:text>return ni::sPlanef::Zero();</xsl:text></xsl:when>
				<xsl:when test="contains($A,'sVec2i')"><xsl:text>return ni::sVec2i::Zero();</xsl:text></xsl:when>
				<xsl:when test="contains($A,'sVec3i')"><xsl:text>return ni::sVec3i::Zero();</xsl:text></xsl:when>
				<xsl:when test="contains($A,'sVec4i')"><xsl:text>return ni::sVec4i::Zero();</xsl:text></xsl:when>
				<xsl:when test="contains($A,'sVec2f')"><xsl:text>return ni::sVec2f::Zero();</xsl:text></xsl:when>
				<xsl:when test="contains($A,'sVec3f')"><xsl:text>return ni::sVec3f::Zero();</xsl:text></xsl:when>
				<xsl:when test="contains($A,'sVec4f')"><xsl:text>return ni::sVec4f::Zero();</xsl:text></xsl:when>
				<xsl:when test="contains($A,'sColor3ub')"><xsl:text>return ni::sColor3ub::Black();</xsl:text></xsl:when>
				<xsl:when test="contains($A,'sColor4ub')"><xsl:text>return ni::sColor4ub::Black();</xsl:text></xsl:when>
				<xsl:when test="contains($A,'sColor3f')"><xsl:text>return ni::sColor3f::Black();</xsl:text></xsl:when>
				<xsl:when test="contains($A,'sColor4f')"><xsl:text>return ni::sColor4f::Black();</xsl:text></xsl:when>
				<xsl:when test="contains($A,'sMatrixf')"><xsl:text>return ni::sMatrixf::Identity();</xsl:text></xsl:when>
				<xsl:when test="contains($A,'sQuatf')"><xsl:text>return ni::sQuatf::Identity();</xsl:text></xsl:when>
				<xsl:when test="contains($A,'Var')"><xsl:text>return niVarNull;</xsl:text></xsl:when>
				<xsl:otherwise>
					<xsl:message terminate='no'>!!!! (get_zero_retvalue) Null implementation cannot return non-constant reference of '<xsl:value-of select="$A"/>' in '<xsl:value-of select="../@name"/>::<xsl:value-of select="@name"/>' by value.</xsl:message>
					<xsl:text>return (</xsl:text><xsl:value-of select="$A"/><xsl:text>)(*(</xsl:text><xsl:value-of select="translate($A,'&amp;','*')"/><xsl:text>)(0));</xsl:text>
				</xsl:otherwise>
				</xsl:choose>
			</xsl:when>
			<!-- Value -->
			<xsl:otherwise>
				<xsl:choose>
					<xsl:when test="&AISFLAGS;"><xsl:text>return ni::eInvalidHandle;</xsl:text></xsl:when>
					<xsl:when test="&AISENUM;"><xsl:text>return (</xsl:text><xsl:value-of select="$A"/><xsl:text>)ni::eInvalidHandle;</xsl:text></xsl:when>
					<xsl:when test="contains($A,'sRecti')"><xsl:text>return ni::sRecti::Null();</xsl:text></xsl:when>
					<xsl:when test="contains($A,'sRectf')"><xsl:text>return ni::sRectf::Null();</xsl:text></xsl:when>
					<xsl:when test="&MATCH_VAR;"><xsl:text>return niVarNull;</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tFVF')"><xsl:text>return 0;</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tType')"><xsl:text>return 0;</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tTime')"><xsl:text>return (tTime)0;</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tVersion')"><xsl:text>return 0;</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tEnum')"><xsl:text>return 0;</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tHandle')"><xsl:text>return 0;</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tOffset')"><xsl:text>return 0;</xsl:text></xsl:when>
					<xsl:when test="contains($A,'cCString')"><xsl:text>return ni::cCString();</xsl:text></xsl:when>
					<xsl:when test="contains($A,'cGString')"><xsl:text>return ni::cGString();</xsl:text></xsl:when>
					<xsl:when test="contains($A,'cXString')"><xsl:text>return ni::cXString();</xsl:text></xsl:when>
					<xsl:when test="contains($A,'cUString')"><xsl:text>return ni::cUString();</xsl:text></xsl:when>
					<xsl:when test="contains($A,'cString')"><xsl:text>return ni::cString();</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tBool')"><xsl:text>return ni::eFalse;</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tSize')"><xsl:text>return 0;</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tI8')"><xsl:text>return 0;</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tU8')"><xsl:text>return 0;</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tI16')"><xsl:text>return 0;</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tU16')"><xsl:text>return 0;</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tI32')"><xsl:text>return 0;</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tU32')"><xsl:text>return 0;</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tI64')"><xsl:text>return 0;</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tU64')"><xsl:text>return 0;</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tUUID')"><xsl:text>return kUUIDZero;</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tInt')"><xsl:text>return 0;</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tUInt')"><xsl:text>return 0;</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tF32')"><xsl:text>return 0;</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tFloat')"><xsl:text>return 0;</xsl:text></xsl:when>
					<xsl:when test="contains($A,'tF64')"><xsl:text>return 0;</xsl:text></xsl:when>
					<xsl:when test="contains($A,'sVec2i')"><xsl:text>return ni::sVec2i::Zero();</xsl:text></xsl:when>
					<xsl:when test="contains($A,'sVec3i')"><xsl:text>return ni::sVec3i::Zero();</xsl:text></xsl:when>
					<xsl:when test="contains($A,'sVec4i')"><xsl:text>return ni::sVec4i::Zero();</xsl:text></xsl:when>
					<xsl:when test="contains($A,'sVec2f')"><xsl:text>return ni::sVec2f::Zero();</xsl:text></xsl:when>
					<xsl:when test="contains($A,'sVec3f')"><xsl:text>return ni::sVec3f::Zero();</xsl:text></xsl:when>
					<xsl:when test="contains($A,'sVec4f')"><xsl:text>return ni::sVec4f::Zero();</xsl:text></xsl:when>
					<xsl:when test="contains($A,'sPlanef')"><xsl:text>return ni::sPlanef(0,0,0,0);</xsl:text></xsl:when>
					<xsl:when test="contains($A,'sColor3ub')"><xsl:text>return ni::sColor3ub::Black();</xsl:text></xsl:when>
					<xsl:when test="contains($A,'sColor4ub')"><xsl:text>return ni::sColor4ub::Black();</xsl:text></xsl:when>
					<xsl:when test="contains($A,'sColor3f')"><xsl:text>return ni::sColor3f::Black();</xsl:text></xsl:when>
					<xsl:when test="contains($A,'sColor4f')"><xsl:text>return ni::sColor4f::Black();</xsl:text></xsl:when>
					<xsl:when test="contains($A,'sQuatf')"><xsl:text>return ni::sQuatf(0,0,0,1);</xsl:text></xsl:when>
					<xsl:when test="contains($A,'sMatrixf')"><xsl:text>return ni::sMatrixf(1);</xsl:text></xsl:when>
					<xsl:otherwise><xsl:text>return NULL;</xsl:text></xsl:otherwise>
				</xsl:choose>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>
</xsl:stylesheet>
