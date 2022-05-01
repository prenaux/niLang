<?xml version="1.0" encoding="Ascii" ?>

<!DOCTYPE stylesheet [
	<!ENTITY SAFEIDLNAME "translate(/niIDL/@name,'.','_')">
	<!ENTITY PARAMETERS "parameter[not(@deprecated='yes')]">
	<!ENTITY METHODS "method[not(contains(@name,'operator')) and not(@name=../@name) and not(@name='niDeclareInterface') and not(@deprecated='yes') and not(@noautomation='yes')]">
	<!ENTITY ALLMETHODS "method[not(contains(@name,'operator')) and not(@name=../@name) and not(@name='niDeclareInterface') and not(@deprecated='yes')]">
	<!ENTITY PARENTS "parents/parent[not(@deprecated='yes')]">
	<!ENTITY INTERFACES "/niIDL/namespace/interface[not(@noautomation='yes') and not(@deprecated='yes')]">
	<!ENTITY EMPTYENUMVALUES "/niIDL/namespace/enum[(@name='') and not(@noautomation='yes') and not(@deprecated='yes')]/value">
	<!ENTITY NAMEDENUMS "/niIDL/namespace/enum[not(@name='') and not(@noautomation='yes') and not(@deprecated='yes') and not(count(value)=0)]">
	<!ENTITY ALLENUMS "/niIDL/namespace/enum[not(@noautomation='yes') and not(@deprecated='yes') and not(count(value)=0)]">
	<!ENTITY OBJECTTYPES "/niIDL/objecttypes/objecttype">
	<!ENTITY AISCONST "(contains($A,'const '))">
	<!ENTITY AISREF "(contains($A,'&amp;'))">
	<!ENTITY AISPTR "(contains($A,'*') and not(&AISPTRPTR;))">
	<!ENTITY AISPTRPTR "(contains($A,'**') or contains($A,'* *'))">
	<!ENTITY AISPTRORREF "(contains($A,'*') or contains($A,'&amp;'))">
	<!ENTITY AISINTERFACE "(starts-with($A,'i') or starts-with($A,'const i') or contains($A,'::i')) and not(starts-with($A,'int'))">
	<!ENTITY AISENUM "(starts-with($A,'e') or starts-with($A,'const e') or contains($A,'::e'))">
	<!ENTITY AISSTRUCT "((starts-with($A,'s') or starts-with($A,'const s') or contains($A,'::s')) and not(contains($A,'sColor3ub') or contains($A,'sColor4ub')))">
	<!ENTITY AISFLAGS "((starts-with($A,'t') or starts-with($A,'const t')) and contains($A,'Flags'))">
	<!ENTITY MATCH_VAR "starts-with($A,'Var') or starts-with($A,'const Var') or starts-with($A,'ni::Var') or starts-with($A,'const ni::Var')">
	<!ENTITY AISVEC "(contains($A,'CVec'))">
	<!ENTITY AISLST "(contains($A,'CLst') or contains($A,'SinkLst'))">
	<!ENTITY AISMAP "(contains($A,'CMap'))">
	<!ENTITY AISCOLLECTION "(&AISVEC; or &AISLST; or &AISMAP;)">
    <!ENTITY AISCHARTYPE "(contains($A,'achar') or contains($A,'cchar') or contains($A,' char') or contains($A,'uchar') or contains($A,'gchar') or contains($A,'xchar'))">
    <!ENTITY SMALLCASE "'abcdefghijklmnopqrstuvwxyz'">
    <!ENTITY UPPERCASE "'ABCDEFGHIJKLMNOPQRSTUVWXYZ'">
	<!ENTITY DISPATCHWRAPPER "((@dispatchwrapper='yes') or (@dispatchwrappercreate) or (starts-with(@name,'i') and contains(@name,'Sink') and not(@name='iSinkList')))">
    <!ENTITY TEST_CANJNI "not(@min_features) or (@min_features &lt;= 20)" >
	<!ENTITY NAME_ISEXCLUDED_INTERFACE "contains(@name,'iUnknown')">
    ]>

<xsl:transform version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	<xsl:include href="GenModuleBase.xslt"/>
	<xsl:output method="text" />

    <!-- ROOT ****************************************************** -->
	<xsl:template match="/">
		<xsl:if test="not(/niIDL/namespace/@name)">
			<xsl:message terminate="yes">No namespace defined !</xsl:message>
		</xsl:if>
		<xsl:call-template name="get_module"/>
	</xsl:template>

    <!-- ************************************************************ -->
	<xsl:template name="get_module">
        <!-- C O N S T A N T S -->

        <!-- E N U M S -->
        <xsl:for-each select="&NAMEDENUMS;">
            <xsl:variable name="NAME" select="concat(translate(substring(@name, 1, 1), &SMALLCASE;, &UPPERCASE;), substring(@name, 2))" />
            <xsl:value-of select="$NAME" /><xsl:text>.java</xsl:text><xsl:call-template name="newline" />
        </xsl:for-each>

        <!-- I N T E R F A C E S -->
        <xsl:for-each select="&INTERFACES;">
            <xsl:if test="&TEST_CANJNI; and not(&NAME_ISEXCLUDED_INTERFACE;)">

                <xsl:variable name="NAME" select="concat(translate(substring(@name, 1, 1), &SMALLCASE;, &UPPERCASE;), substring(@name, 2))" />
                <xsl:variable name="NAMEIMPL" select="concat(translate(substring(@name, 1, 1), &SMALLCASE;, &UPPERCASE;), substring(@name, 2), 'Impl')" />

                <xsl:value-of select="$NAME" /><xsl:text>.java</xsl:text><xsl:call-template name="newline" />

                <xsl:if test="&DISPATCHWRAPPER;">
                    <xsl:value-of select="$NAMEIMPL" /><xsl:text>.java</xsl:text><xsl:call-template name="newline" />
                </xsl:if>
            </xsl:if>
        </xsl:for-each> <!-- for-each INTERFACES -->
    </xsl:template>

    <!-- ************************************************************ -->
    <!-- *** E O F ************************************************** -->
    <!-- ************************************************************ -->
</xsl:transform>
