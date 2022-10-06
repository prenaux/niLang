<?xml version="1.0" encoding="Ascii" ?>
<!DOCTYPE stylesheet [
  <!ENTITY SAFEIDLNAME "translate(/niIDL/@name,'.-','__')">
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
  <!ENTITY AISSMARTPTR "contains($A,'Ptr&lt;')">
  <!ENTITY AISPTRPTR "(contains($A,'**') or contains($A,'* *'))">
  <!ENTITY AISPTRORREF "(contains($A,'*') or contains($A,'Ptr&lt;') or contains($A,'&amp;'))">
  <!ENTITY AISINTERFACE "(starts-with($A,'i') or starts-with($A,'const i') or contains($A,'::i') or contains($A,'Ptr&lt;')) and not(starts-with($A,'int'))">
  <!ENTITY AISENUM "(starts-with($A,'e') or starts-with($A,'const e') or contains($A,'::e'))">
  <!ENTITY AISSTRUCT "((starts-with($A,'s') or starts-with($A,'const s') or contains($A,'::s')) and not(contains($A,'sColor3ub') or contains($A,'sColor4ub')))">
  <!ENTITY AISFLAGS "((starts-with($A,'t') or starts-with($A,'const t') or contains($A,'::t')) and contains($A,'Flags'))">
  <!ENTITY MATCH_VAR "starts-with($A,'Var') or starts-with($A,'const Var') or starts-with($A,'ni::Var') or starts-with($A,'const ni::Var')">
  <!ENTITY AISVEC "(contains($A,'CVec'))">
  <!ENTITY AISSINKLIST "(contains($A,'iSinkLst') or contains($A,'SinkLst'))">
  <!ENTITY AISLST "(contains($A,'CLst') or &AISSINKLIST;)">
  <!ENTITY AISMAP "(contains($A,'CMap'))">
  <!ENTITY AISCOLLECTION "(&AISVEC; or &AISLST; or &AISMAP;)">
    <!ENTITY AISCHARTYPE "(contains($A,'achar') or contains($A,'cchar') or contains($A,' char') or contains($A,'uchar') or contains($A,'gchar') or contains($A,'xchar'))">
    <!ENTITY SMALLCASE "'abcdefghijklmnopqrstuvwxyz'">
    <!ENTITY UPPERCASE "'ABCDEFGHIJKLMNOPQRSTUVWXYZ'">
  <!ENTITY DISPATCHWRAPPER "((@dispatchwrapper='yes') or (@dispatchwrappercreate) or (starts-with(@name,'i') and contains(@name,'Sink') and not(@name='iSinkList')))">
    <!ENTITY TEST_CANJNI "(not(@min_features) or (@min_features &lt;= 20))" >
  <!ENTITY NAME_ISEXCLUDED_INTERFACE "contains(@name,'iUnknown')">
]>

<xsl:transform version="2.0"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

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
            <xsl:variable name="PKGNAME" select="/niIDL/@name" />
            <xsl:variable name="NAME">
                <xsl:call-template name="get_type_class">
                    <xsl:with-param name="A" select="@name" />
                </xsl:call-template>
            </xsl:variable>

            <xsl:text>ni.</xsl:text><xsl:value-of select="$PKGNAME"/><xsl:text>.</xsl:text><xsl:value-of select="$NAME"/><xsl:call-template name="newline"/>

            <xsl:result-document href="{$NAME}.java" method="text">
                <xsl:call-template name="get_java_header"/>

<xsl:text>
/**
 * </xsl:text><xsl:value-of select="@name" /><xsl:text>
 * </xsl:text><xsl:if test="not(string-length(comments/@desc)=0)"><xsl:text>
 * </xsl:text><xsl:value-of select="comments/@desc"/></xsl:if>
   <xsl:value-of select="comments/remark/@text" /><xsl:text>
 */
@SuppressWarnings("unused")
public final class </xsl:text><xsl:value-of select="$NAME"/><xsl:text> {
  private </xsl:text><xsl:value-of select="$NAME"/><xsl:text>() {
  }

  public static final String NAME = &quot;</xsl:text><xsl:value-of select="@name" /><xsl:text>&quot;;

</xsl:text>

            <xsl:for-each select="value">
                <xsl:if test="&TEST_CANJNI;">
                    <xsl:variable name="E" select="substring-after(@name,concat(../@name,'_'))" />
                    <xsl:variable name="ENAME">
                        <xsl:choose>
                            <xsl:when test="starts-with($E,'0') or starts-with($E,'1') or starts-with($E,'2') or starts-with($E,'3') or starts-with($E,'4') or starts-with($E,'5') or starts-with($E,'6') or starts-with($E,'7') or starts-with($E,'8') or starts-with($E,'9')">
                                <xsl:value-of select="concat('_',$E)" />
                            </xsl:when>
                            <xsl:otherwise>
                                <xsl:value-of select="$E" />
                            </xsl:otherwise>
                        </xsl:choose>
                    </xsl:variable>
                    <xsl:variable name="VAL">
                        <xsl:if test="@eval_value">
                            <xsl:value-of select="@eval_value" />
                        </xsl:if>
                        <xsl:if test="not(@eval_value)">
                            <xsl:call-template name="get_enum_value">
                                <xsl:with-param name="VALUE">
                                    <xsl:choose>
                                        <xsl:when test="contains(@value,' : : ')">
                                            <xsl:value-of select="substring-after(@value,' : : ')" />
                                        </xsl:when>
                                        <xsl:when test="contains(@value,'::')">
                                            <xsl:value-of select="substring-after(@value,'::')" />
                                        </xsl:when>
                                        <xsl:otherwise>
                                            <xsl:value-of select="@value" />
                                        </xsl:otherwise>
                                    </xsl:choose>
                                </xsl:with-param>
                                <xsl:with-param name="ENUM">
                                    <xsl:call-template name="get_type_class">
                                        <xsl:with-param name="A" select="../@name" />
                                    </xsl:call-template>
                                </xsl:with-param>
                            </xsl:call-template>
                        </xsl:if>
                    </xsl:variable>
<xsl:text>
  /**
   * </xsl:text><xsl:value-of select="substring-after(@name,concat(../@name,'_'))"/><xsl:text> = </xsl:text><xsl:value-of select="translate(normalize-space(@value),' ','')"/>
<xsl:if test="not(string-length(comments/@desc)=0)"><xsl:text>
   * </xsl:text><xsl:value-of select="comments/@desc"/></xsl:if><xsl:value-of select="./comments/remark/@text" />
<xsl:text>
   */
  public final static int </xsl:text><xsl:value-of select="$ENAME"/><xsl:text> = </xsl:text><xsl:value-of select="$VAL"/><xsl:text>;
</xsl:text>
                </xsl:if>
            </xsl:for-each>

<xsl:text>
}

</xsl:text>
                </xsl:result-document>
        </xsl:for-each>

        <!-- I N T E R F A C E S -->
        <xsl:for-each select="&INTERFACES;">
            <xsl:if test="&TEST_CANJNI; and not(&NAME_ISEXCLUDED_INTERFACE;)">

            <xsl:variable name="PKGNAME" select="/niIDL/@name" />
            <xsl:variable name="PARENTNAME">
                <xsl:call-template name="get_type_class">
                    <xsl:with-param name="A" select="parents/parent/@name" />
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="NAME">
                <xsl:call-template name="get_type_class">
                    <xsl:with-param name="A" select="@name" />
                </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="NAMEIMPL">
                <xsl:call-template name="get_type_class">
                    <xsl:with-param name="A" select="@name" />
                </xsl:call-template><xsl:text>Impl</xsl:text>
            </xsl:variable>

            <xsl:text>ni.</xsl:text><xsl:value-of select="$PKGNAME"/><xsl:text>.</xsl:text><xsl:value-of select="$NAME"/><xsl:call-template name="newline"/>

            <xsl:result-document href="{$NAME}.java" method="text">
                <xsl:call-template name="get_java_header"/>

                <xsl:text>/**</xsl:text><xsl:call-template name="newline"/>
                <xsl:text> * </xsl:text><xsl:value-of select="@name"/><xsl:text> interface</xsl:text><xsl:call-template name="newline"/>
                <xsl:text> */ </xsl:text><xsl:call-template name="newline"/>
                <xsl:text>@SuppressWarnings("unused")</xsl:text><xsl:call-template name="newline"/>
                <xsl:text>@Platform(include={&quot;stdafx.h&quot;})</xsl:text><xsl:call-template name="newline"/>
                <xsl:text>@Namespace(&quot;</xsl:text><xsl:value-of select="/niIDL/namespace[1]/@name"/><xsl:text>&quot;)</xsl:text><xsl:call-template name="newline"/>
                <xsl:text>@Name(&quot;</xsl:text><xsl:value-of select="@name"/><xsl:text>&quot;)</xsl:text><xsl:call-template name="newline"/>
                <xsl:text>public class </xsl:text><xsl:value-of select="$NAME"/><xsl:text> extends </xsl:text><xsl:value-of select="$PARENTNAME"/>
                <xsl:if test="&DISPATCHWRAPPER;">
                    <xsl:text> implements </xsl:text><xsl:value-of select="$NAMEIMPL"/>
                </xsl:if>
                <xsl:text> {</xsl:text>

                <xsl:call-template name="newline"/>
                <xsl:text>  public static final UUID UUID = new UUID(</xsl:text><xsl:value-of select="@uuid2"/><xsl:text>);</xsl:text><xsl:call-template name="newline"/>
                <xsl:call-template name="newline"/>
                <xsl:text>  protected </xsl:text><xsl:value-of select="$NAME"/><xsl:text>() {};</xsl:text><xsl:call-template name="newline"/>
                <xsl:call-template name="newline"/>
                <xsl:text>  /**</xsl:text><xsl:call-template name="newline"/>
                <xsl:text>   * Query this interface on the specified object.</xsl:text><xsl:call-template name="newline"/>
                <xsl:text>   */</xsl:text><xsl:call-template name="newline"/>
                <xsl:text>  @Static(name=&quot;niJVM_QueryInterface&lt;</xsl:text><xsl:value-of select="/niIDL/namespace[1]/@name"/>::<xsl:value-of select="@name"/><xsl:text>&gt;&quot;)</xsl:text><xsl:call-template name="newline"/>
                <xsl:text>  public static native </xsl:text><xsl:value-of select="$NAME"/><xsl:text> query(Object obj);</xsl:text><xsl:call-template name="newline"/>
                <xsl:call-template name="newline"/>
                <xsl:text>  /**</xsl:text><xsl:call-template name="newline"/>
                <xsl:text>   * Query this interface on the specified IUnknown.</xsl:text><xsl:call-template name="newline"/>
                <xsl:text>   */</xsl:text><xsl:call-template name="newline"/>
                <xsl:text>  @Static(name=&quot;niJVM_QueryInterface&lt;</xsl:text><xsl:value-of select="/niIDL/namespace[1]/@name"/>::<xsl:value-of select="@name"/><xsl:text>&gt;&quot;)</xsl:text><xsl:call-template name="newline"/>
                <xsl:text>  public static native </xsl:text><xsl:value-of select="$NAME"/><xsl:text> query(IUnknown obj);</xsl:text><xsl:call-template name="newline"/>
                <xsl:call-template name="newline"/>

                <xsl:call-template name="newline"/>
                <xsl:for-each select="&METHODS;">
                    <!-- M E T H O D S -->
                    <xsl:if test="&TEST_CANJNI;">
                    <xsl:variable name="MNAME" select="@java_name" />
                    <xsl:text>  /**</xsl:text><xsl:call-template name="newline"/>
                    <xsl:text>   * </xsl:text>
                    <xsl:if test="not(string-length(comments/@desc)=0)"><xsl:value-of select="comments/@desc"/></xsl:if>
                    <xsl:value-of select="./comments/remark/@text" />
                    <xsl:call-template name="newline"/>
                    <xsl:for-each select="parameter">
                        <xsl:text>   * @param </xsl:text><xsl:value-of select="@name" />
                        <xsl:text> {</xsl:text>
                        <xsl:call-template name="get_doc_type"><xsl:with-param name="A" select="@type" /></xsl:call-template>
                        <xsl:text>} </xsl:text>
                        <xsl:call-template name="newline"/>
                    </xsl:for-each> <!-- for-each parameter -->
                    <xsl:text>   * @return {</xsl:text>
                    <xsl:call-template name="get_doc_type"><xsl:with-param name="A" select="return/@type" /></xsl:call-template>
                    <xsl:text>}</xsl:text>
                    <xsl:value-of select="./comments/@return" />
                    <xsl:call-template name="newline"/>
                    <xsl:text>   */</xsl:text>
                    <xsl:call-template name="newline"/>
                    <xsl:text>  @Name(value=&quot;</xsl:text>
                    <xsl:value-of select="@name" />
                    <xsl:text>&quot;)</xsl:text>
                    <xsl:if test="@const='yes'"> @ConstMeth</xsl:if>
                    <xsl:call-template name="newline"/>
                    <xsl:text>  public native </xsl:text>
                    <xsl:call-template name="get_java_return_type"><xsl:with-param name="A" select="return/@type" /></xsl:call-template>
                    <xsl:text> </xsl:text>
                    <xsl:value-of select="$MNAME" />
                    <xsl:text>(</xsl:text>
                    <xsl:for-each select="parameter">
                        <xsl:call-template name="get_java_type"><xsl:with-param name="A" select="@type" /></xsl:call-template>
                        <xsl:text> </xsl:text>
                        <xsl:value-of select="@name" />
                        <xsl:if test="not(@last='1')"><xsl:text>, </xsl:text></xsl:if>
                    </xsl:for-each> <!-- for-each parameter -->
                    <xsl:text>);</xsl:text>
                    <xsl:call-template name="newline"/>
                    </xsl:if>
                </xsl:for-each> <!-- for-each METHODS -->

                <xsl:if test="&DISPATCHWRAPPER;">
                    <xsl:call-template name="newline"/>
                    <xsl:text>  /**</xsl:text><xsl:call-template name="newline"/>
                    <xsl:text>   * Creates a native implementation which wraps a JVM implementation so that it can be used by native methods.</xsl:text><xsl:call-template name="newline"/>
                    <xsl:text>   */</xsl:text><xsl:call-template name="newline"/>
                    <xsl:text>  public static native </xsl:text><xsl:value-of select="$NAME"/><xsl:text> impl(</xsl:text><xsl:value-of select="$NAMEIMPL"/><xsl:text> impl);</xsl:text>
                    <xsl:call-template name="newline"/>
                </xsl:if>

                <xsl:text>}</xsl:text><xsl:call-template name="newline"/>
            </xsl:result-document>

            <xsl:if test="&DISPATCHWRAPPER;">
                <xsl:text>ni.</xsl:text><xsl:value-of select="$PKGNAME"/><xsl:text>.</xsl:text><xsl:value-of select="$NAMEIMPL"/><xsl:call-template name="newline"/>

                <xsl:result-document href="{$NAMEIMPL}.java" method="text">
                    <xsl:call-template name="get_java_header"/>
                    <xsl:text>@SuppressWarnings("unused")</xsl:text><xsl:call-template name="newline"/>
                    <xsl:text>@Platform(include={&quot;stdafx.h&quot;})</xsl:text><xsl:call-template name="newline"/>
                    <xsl:text>@Namespace(&quot;</xsl:text><xsl:value-of select="/niIDL/namespace[1]/@name"/><xsl:text>&quot;)</xsl:text><xsl:call-template name="newline"/>
                    <xsl:text>@Name(&quot;</xsl:text><xsl:value-of select="@name"/><xsl:text>&quot;)</xsl:text><xsl:call-template name="newline"/>
                    <xsl:text>@ImplWrapper()</xsl:text><xsl:call-template name="newline"/>
                    <xsl:text>public interface </xsl:text><xsl:value-of select="$NAMEIMPL"/><xsl:text> {</xsl:text><xsl:call-template name="newline"/>
                    <xsl:for-each select="&METHODS;">
                        <xsl:if test="&TEST_CANJNI;">
                            <!-- M E T H O D S -->
                            <xsl:variable name="MNAME" select="@java_name" />
                            <xsl:text>  @Name(value=&quot;</xsl:text>
                            <xsl:value-of select="@name" />
                            <xsl:text>&quot;)</xsl:text>
                            <xsl:if test="@const='yes'"><xsl:text>  @ConstMeth</xsl:text></xsl:if>
                            <xsl:call-template name="newline"/>
                            <xsl:text>  public </xsl:text>
                            <xsl:call-template name="get_java_sink_return_type"><xsl:with-param name="A" select="return/@type" /></xsl:call-template>
                            <xsl:text> </xsl:text>
                            <xsl:value-of select="$MNAME" />
                            <xsl:text>(</xsl:text>
                            <xsl:for-each select="parameter">
                                <xsl:call-template name="get_java_type"><xsl:with-param name="A" select="@type" /></xsl:call-template>
                                <xsl:text> </xsl:text>
                                <xsl:value-of select="@name" />
                                <xsl:if test="not(@last='1')"><xsl:text>, </xsl:text></xsl:if>
                            </xsl:for-each> <!-- for-each parameter -->
                            <xsl:text>);</xsl:text>
                            <xsl:call-template name="newline"/>
                        </xsl:if>
                    </xsl:for-each> <!-- for-each METHODS -->

                    <xsl:text>}</xsl:text><xsl:call-template name="newline"/>
                </xsl:result-document>
            </xsl:if>
            </xsl:if>
        </xsl:for-each> <!-- for-each INTERFACES -->
    </xsl:template>

<!-- ************************************************************
* Get the Java Class name
************************************************************* -->
  <xsl:template name="get_type_class">
    <xsl:param name="A" />

        <xsl:variable name="R">
            <xsl:choose>
                <xsl:when test="not($A)">void</xsl:when>
                <xsl:when test="&AISENUM;">
                    <xsl:text>e</xsl:text>
                    <xsl:choose>
                        <xsl:when test="contains($A,'::e')">
                            <xsl:value-of select="substring-after($A,'::e')" />
                        </xsl:when>
                        <xsl:when test="starts-with($A,'const ')">
                            <xsl:value-of select="substring-after($A,' e')" />
                        </xsl:when>
                        <xsl:otherwise>
                            <xsl:value-of select="substring-after($A,'e')" />
                        </xsl:otherwise>
                    </xsl:choose>
                </xsl:when>

                <xsl:when test="&AISSTRUCT;">
                    <xsl:text>s</xsl:text>
                    <xsl:choose>
                        <xsl:when test="contains($A,'::s')">
                            <xsl:value-of select="substring-after($A,'::s')" />
                        </xsl:when>
                        <xsl:when test="starts-with($A,'const ')">
                            <xsl:value-of select="substring-after($A,' s')" />
                        </xsl:when>
                        <xsl:otherwise>
                            <xsl:value-of select="substring-after($A,'s')" />
                        </xsl:otherwise>
                    </xsl:choose>
                </xsl:when>

                <xsl:when test="&AISFLAGS;">
                    <xsl:variable name="ENUMNAME_TEMP">
                        <xsl:choose>
                            <xsl:when test="contains($A,'::t')">
                                <xsl:text>e</xsl:text>
                                <xsl:value-of select="substring-after($A,'::t')" />
                            </xsl:when>
                            <xsl:when test="starts-with($A,'const ')">
                                <xsl:value-of select="substring-after($A,' t')" />
                            </xsl:when>
                            <xsl:otherwise>
                                <xsl:text>e</xsl:text>
                                <xsl:value-of select="substring-after($A,'t')" />
                            </xsl:otherwise>
                        </xsl:choose>
                    </xsl:variable>
                    <xsl:variable name="ENUMNAME">
                        <xsl:choose>
                            <xsl:when test="/niIDL/namespace/enum[@name=$ENUMNAME_TEMP]/@name">
                                <xsl:value-of select="$ENUMNAME_TEMP" />
                            </xsl:when>
                            <xsl:when test="/niIDL/namespace/enum[@name=substring-before($ENUMNAME_TEMP,'Flags')]/@name">
                                <xsl:value-of select="substring-before($ENUMNAME_TEMP,'Flags')" />
                            </xsl:when>
                            <xsl:otherwise>
                                <xsl:value-of select="$ENUMNAME_TEMP" />
                            </xsl:otherwise>
                        </xsl:choose>
                    </xsl:variable>

                    <xsl:text></xsl:text>
                    <xsl:if test="contains($A,'::t')">
                        <xsl:value-of select="substring-before($A,'::t')" />
                        <xsl:text>::</xsl:text>
                    </xsl:if>
                    <xsl:value-of select="$ENUMNAME" />
                </xsl:when>

                <xsl:when test="&AISCOLLECTION;"><xsl:text>iCollection</xsl:text></xsl:when>

                <xsl:when test="&AISINTERFACE;">
                    <xsl:choose>
                        <xsl:when test="contains($A,'Ptr&lt;')">
                            <xsl:call-template name="get_without_namespace">
                              <xsl:with-param name="A">
                                <xsl:value-of select="substring-after(substring-before($A,'&gt;'),'&lt;')"/>
                              </xsl:with-param>
                            </xsl:call-template>
                        </xsl:when>
                        <xsl:when test="starts-with($A,'const ') and contains($A,' *')">
                            <xsl:call-template name="get_without_namespace"><xsl:with-param name="A"><xsl:value-of select="normalize-space(substring($A,7,string-length(substring-before($A,' *'))-5))"/></xsl:with-param></xsl:call-template>
                        </xsl:when>
                        <xsl:when test="contains($A,' *')">
                            <xsl:call-template name="get_without_namespace"><xsl:with-param name="A"><xsl:value-of select="normalize-space(substring($A,1,string-length(substring-before($A,' *'))))"/></xsl:with-param></xsl:call-template>
                        </xsl:when>
                        <xsl:when test="starts-with($A,'const ') and contains($A,'*')">
                            <xsl:call-template name="get_without_namespace"><xsl:with-param name="A"><xsl:value-of select="normalize-space(substring($A,7,string-length(substring-before($A,'*'))-5))"/></xsl:with-param></xsl:call-template>
                        </xsl:when>
                        <xsl:when test="contains($A,'*')">
                            <xsl:call-template name="get_without_namespace"><xsl:with-param name="A"><xsl:value-of select="normalize-space(substring($A,1,string-length(substring-before($A,'*'))))"/></xsl:with-param></xsl:call-template>
                        </xsl:when>
                        <xsl:when test="starts-with($A,'const ')">
                            <xsl:call-template name="get_without_namespace"><xsl:with-param name="A"><xsl:value-of select="normalize-space(substring($A,7,string-length($A)))"/></xsl:with-param></xsl:call-template>
                        </xsl:when>
                        <xsl:otherwise>
                            <xsl:call-template name="get_without_namespace"><xsl:with-param name="A"><xsl:value-of select="normalize-space(substring($A,1))"/></xsl:with-param></xsl:call-template>
                        </xsl:otherwise>
                    </xsl:choose>
                </xsl:when>

                <xsl:when test="starts-with($A,'const ')">
                    <xsl:call-template name="get_without_namespace"><xsl:with-param name="A"><xsl:value-of select="normalize-space(substring($A,7,string-length(substring-before($A,' *'))-5))"/></xsl:with-param></xsl:call-template>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:call-template name="get_without_namespace"><xsl:with-param name="A"><xsl:value-of select="normalize-space(substring($A,1,string-length(substring-before($A,' *'))))"/></xsl:with-param></xsl:call-template>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:variable>

        <xsl:variable name="R1" select="normalize-space(concat(translate(substring($R, 1, 1), &SMALLCASE;, &UPPERCASE;), substring($R, 2)))" />
        <xsl:if test="contains($R1,' ')">
            <xsl:value-of select="substring-before($R1,' ')" />
        </xsl:if>
        <xsl:if test="not(contains($R1,' '))">
            <xsl:value-of select="$R1" />
        </xsl:if>
  </xsl:template>

<!-- ************************************************************
* Get the Java annotation
************************************************************* -->
  <xsl:template name="get_java_annotations">
    <xsl:param name="A" />
    <xsl:variable name="R">
      <xsl:if test="&AISCONST;">@Const </xsl:if>
      <xsl:if test="&AISPTRPTR;">@ByPtrPtr </xsl:if>
      <xsl:if test="&AISPTR;">@ByPtr </xsl:if>
      <xsl:if test="&AISREF;">@ByRef </xsl:if>
    </xsl:variable>
    <xsl:value-of select="$R" />
  </xsl:template>

<!-- ************************************************************
* Get the Java type
************************************************************* -->
    <xsl:template name="get_java_return_type">
        <xsl:param name="A" />
        <xsl:param name="B" />

        <xsl:variable name="R">
            <xsl:call-template name="get_java_type">
                <xsl:with-param name="A" select="$A" />
                <xsl:with-param name="B" select="$B" />
            </xsl:call-template>
        </xsl:variable>

        <xsl:value-of select="$R" />
    </xsl:template>

    <xsl:template name="get_java_sink_return_type">
        <xsl:param name="A" />
        <xsl:param name="B" />

        <xsl:variable name="R">
            <xsl:choose>
                <xsl:when test="&MATCH_VAR; and &AISPTRORREF;">
                    <xsl:message>E/ Invalid Sink Return Type : <xsl:value-of select="$A"/> in <xsl:value-of select="../../@name"/>::<xsl:value-of select="../@name"/></xsl:message>
                </xsl:when>
                <xsl:when test="&AISCHARTYPE;">
                    <xsl:text>@Ret(prefix=&quot;niJVM_GetWrapperRetChars(e,&quot;,suffix=&quot;)&quot;) </xsl:text>
                </xsl:when>
            </xsl:choose>
            <xsl:call-template name="get_java_type">
                <xsl:with-param name="A" select="$A" />
                <xsl:with-param name="B" select="$B" />
            </xsl:call-template>
        </xsl:variable>

        <xsl:value-of select="$R" />
    </xsl:template>

    <xsl:template name="get_java_type">
        <xsl:param name="A" />
        <xsl:param name="B" />

        <xsl:variable name="ANNO">
          <xsl:if test="not(&AISCHARTYPE; or &AISINTERFACE; or &AISCOLLECTION; or contains($A,'cString'))">
            <xsl:call-template name="get_java_annotations">
              <xsl:with-param name="A" select="$A" />
            </xsl:call-template>
          </xsl:if>
          <xsl:if test="&AISINTERFACE; and &AISSMARTPTR;">
            <xsl:text>@BySmartPtr </xsl:text>
          </xsl:if>
          <xsl:if test="contains($A,'cString') and (not(&AISREF;) or &AISCONST;)">
            <xsl:text>@ByString </xsl:text>
          </xsl:if>
        </xsl:variable>

        <xsl:variable name="R">
            <xsl:choose>
                <xsl:when test="&AISPTRPTR;">
                    <xsl:message>E/ Invalid Pointer Pointer Type : <xsl:value-of select="$A"/> in <xsl:value-of select="../../@name"/>::<xsl:value-of select="../@name"/></xsl:message>
                    <xsl:text>&lt;Invalid - PointerPointer&gt;</xsl:text>
                </xsl:when>
                <xsl:when test="not(&AISCONST;) and &AISSINKLIST;">
                    <xsl:text>@Cast(&quot;</xsl:text><xsl:value-of select="$A"/><xsl:text>&quot;) ISinkList</xsl:text>
                </xsl:when>
                <xsl:when test="&AISCONST; and &AISCOLLECTION;">
                    <xsl:text>@Const </xsl:text>
                    <xsl:text>@Cast(&quot;</xsl:text><xsl:value-of select="$A"/><xsl:text>&quot;) ICollection</xsl:text>
                </xsl:when>
                <xsl:when test="not(&AISCONST;) and &AISCOLLECTION;">
                    <xsl:text>@Cast(&quot;</xsl:text><xsl:value-of select="$A"/><xsl:text>&quot;) IMutableCollection</xsl:text>
                </xsl:when>
                <xsl:when test="&AISINTERFACE;">
                    <xsl:if test="&AISCONST;"><xsl:text>@Const </xsl:text></xsl:if>
                    <xsl:call-template name="get_type_class"><xsl:with-param name="A"><xsl:value-of select="$A"/></xsl:with-param></xsl:call-template>
                </xsl:when>
                <xsl:when test="&AISENUM;">
                    <xsl:text>@Cast(&quot;</xsl:text><xsl:value-of select="$A"/><xsl:text>&quot;) @Unsigned int</xsl:text>
                </xsl:when>
                <xsl:when test="&AISFLAGS;">@Unsigned int</xsl:when>
                <!-- general cases -->
                <xsl:when test="contains($A,'sVec2i')"><xsl:text>Vec2i</xsl:text></xsl:when>
                <xsl:when test="contains($A,'sVec3i')"><xsl:text>Vec3i</xsl:text></xsl:when>
                <xsl:when test="contains($A,'sVec4i')"><xsl:text>Vec4i</xsl:text></xsl:when>
                <xsl:when test="contains($A,'sVec2f')"><xsl:text>Vec2f</xsl:text></xsl:when>
                <xsl:when test="contains($A,'sVec3f')"><xsl:text>Vec3f</xsl:text></xsl:when>
                <xsl:when test="contains($A,'sVec4f')"><xsl:text>Vec4f</xsl:text></xsl:when>
                <xsl:when test="contains($A,'sPlanef')"><xsl:text>Vec4f</xsl:text></xsl:when>
                <xsl:when test="contains($A,'sColor3f')"><xsl:text>Vec3f</xsl:text></xsl:when>
                <xsl:when test="contains($A,'sColor4f')"><xsl:text>Vec4f</xsl:text></xsl:when>
                <xsl:when test="contains($A,'sColor3ub')"><xsl:text>Color3ub</xsl:text></xsl:when>
                <xsl:when test="contains($A,'sColor4ub')"><xsl:text>Color4ub</xsl:text></xsl:when>
                <xsl:when test="contains($A,'sQuatf')"><xsl:text>Vec4f</xsl:text></xsl:when>
                <xsl:when test="contains($A,'sMatrixf')"><xsl:text>Matrixf</xsl:text></xsl:when>
                <xsl:when test="contains($A,'sRecti')"><xsl:text>Vec4i</xsl:text></xsl:when>
                <xsl:when test="contains($A,'sRectf')"><xsl:text>Vec4f</xsl:text></xsl:when>
                <xsl:when test="contains($A,'tUUID')"><xsl:text>UUID</xsl:text></xsl:when>
                <xsl:when test="contains($A,'sUUID')"><xsl:text>UUID</xsl:text></xsl:when>
                <xsl:when test="&MATCH_VAR;"><xsl:text>Object</xsl:text></xsl:when>
                <xsl:when test="&AISPTR; and contains($A,'void')"><xsl:text>@Cast(&quot;void*&quot;) Buffer</xsl:text></xsl:when>
                <xsl:when test="&AISPTR; and contains($A,'tI8')"><xsl:text>@Cast(&quot;ni::tI8*&quot;) ByteBuffer</xsl:text></xsl:when>
                <xsl:when test="&AISPTR; and contains($A,'tU8')"><xsl:text>@Cast(&quot;ni::tU8*&quot;) ByteBuffer</xsl:text></xsl:when>
                <xsl:when test="&AISPTR; and contains($A,'tI16')"><xsl:text>@Cast(&quot;ni::tI16*&quot;) ShortBuffer</xsl:text></xsl:when>
                <xsl:when test="&AISPTR; and contains($A,'tU16')"><xsl:text>@Cast(&quot;ni::tU16*&quot;) ShortBuffer</xsl:text></xsl:when>
                <xsl:when test="&AISPTR; and contains($A,'tI32')"><xsl:text>@Cast(&quot;ni::tI32*&quot;) IntBuffer</xsl:text></xsl:when>
                <xsl:when test="&AISPTR; and (contains($A,'tU32') or contains($A,'tSize'))"><xsl:text>@Cast(&quot;ni::tU32*&quot;) IntBuffer</xsl:text></xsl:when>
                <xsl:when test="&AISPTR; and contains($A,'tI64')"><xsl:text>@Cast(&quot;ni::tI64*&quot;) LongBuffer</xsl:text></xsl:when>
                <xsl:when test="&AISPTR; and contains($A,'tU64')"><xsl:text>@Cast(&quot;ni::tU64*&quot;) LongBuffer</xsl:text></xsl:when>
                <xsl:when test="&AISPTR; and (contains($A,'tF32') or contains($A,'tFloat'))"><xsl:text>@Cast(&quot;ni::tF32*&quot;) FloatBuffer</xsl:text></xsl:when>
                <xsl:when test="&AISPTR; and contains($A,'tF64')"><xsl:text>@Cast(&quot;ni::tF64*&quot;) DoubleBuffer</xsl:text></xsl:when>
                <xsl:when test="&AISSTRUCT;">
                    <xsl:call-template name="get_type_class"><xsl:with-param name="A"><xsl:value-of select="$A"/></xsl:with-param></xsl:call-template>
                </xsl:when>
                <xsl:when test="&AISCHARTYPE;">
                    <xsl:if test="not(starts-with($A,'const') and &AISPTR;)">
                        <xsl:message>E/ Invalid Char Type : <xsl:value-of select="$A"/> in <xsl:value-of select="../../@name"/>::<xsl:value-of select="../@name"/></xsl:message>
                        <xsl:text>&lt;Invalid - Char Type&gt;</xsl:text>
                    </xsl:if>
                    <xsl:choose>
                        <xsl:when test="contains($A,'achar')"><xsl:text>String</xsl:text></xsl:when>
                        <xsl:when test="contains($A,'cchar') or contains($A,' char')"><xsl:text>String</xsl:text></xsl:when>
                        <xsl:when test="contains($A,'uchar')"><xsl:text>String</xsl:text></xsl:when>
                        <xsl:when test="contains($A,'gchar')"><xsl:text>String</xsl:text></xsl:when>
                        <xsl:when test="contains($A,'xchar')"><xsl:text>String</xsl:text></xsl:when>
                    </xsl:choose>
                </xsl:when>
                <xsl:when test="&AISPTR;">
                    <xsl:message>E/ Invalid Pointer Type : <xsl:value-of select="$A"/> in <xsl:value-of select="../../@name"/>::<xsl:value-of select="../@name"/></xsl:message>
                    <xsl:text>&lt;Invalid - Pointer&gt;</xsl:text>
                </xsl:when>
                <xsl:when test="contains($A,'cString') and (not(&AISREF;) or &AISCONST;)"><xsl:text>String</xsl:text></xsl:when>
                <xsl:when test="&AISREF;">
                    <xsl:message>E/ Invalid Reference Type : <xsl:value-of select="$A"/> in <xsl:value-of select="../../@name"/>::<xsl:value-of select="../@name"/></xsl:message>
                    <xsl:text>&lt;Invalid - Reference&gt;</xsl:text>
                </xsl:when>
                <xsl:when test="starts-with($A,'void')"><xsl:text>void</xsl:text></xsl:when>
                <xsl:when test="contains($A,'tI8')"><xsl:text>byte</xsl:text></xsl:when>
                <xsl:when test="contains($A,'tU8')"><xsl:text>@Unsigned byte</xsl:text></xsl:when>
                <xsl:when test="contains($A,'tI16')"><xsl:text>short</xsl:text></xsl:when>
                <xsl:when test="contains($A,'tU16')"><xsl:text>@Unsigned short</xsl:text></xsl:when>
                <xsl:when test="contains($A,'tI32')"><xsl:text>int</xsl:text></xsl:when>
                <xsl:when test="contains($A,'tU32')"><xsl:text>@Unsigned int</xsl:text></xsl:when>
                <xsl:when test="contains($A,'tI64')"><xsl:text>long</xsl:text></xsl:when>
                <xsl:when test="contains($A,'tU64')"><xsl:text>@Unsigned long</xsl:text></xsl:when>
                <xsl:when test="contains($A,'tF32') or contains($A,'tFloat')"><xsl:text>float</xsl:text></xsl:when>
                <xsl:when test="contains($A,'tF64')"><xsl:text>double</xsl:text></xsl:when>
                <xsl:when test="contains($A,'tFVF')"><xsl:text>@Unsigned int</xsl:text></xsl:when>
                <xsl:when test="contains($A,'tType')"><xsl:text>@Unsigned int</xsl:text></xsl:when>
                <xsl:when test="contains($A,'tTime')"><xsl:text>long</xsl:text></xsl:when>
                <xsl:when test="contains($A,'tVersion')"><xsl:text>@Unsigned int</xsl:text></xsl:when>
                <xsl:when test="contains($A,'tHandle')"><xsl:text>@Cast(&quot;ni::tHandle&quot;) long</xsl:text></xsl:when>
                <xsl:when test="contains($A,'tIntPtr')"><xsl:text>@Cast(&quot;ni::tIntPtr&quot;) long</xsl:text></xsl:when>
                <xsl:when test="contains($A,'tUIntPtr')"><xsl:text>@Cast(&quot;ni::tUIntPtr&quot;) long</xsl:text></xsl:when>
                <xsl:when test="contains($A,'tPtr')"><xsl:text>@Cast(&quot;ni::tPtr&quot;) Pointer</xsl:text></xsl:when>
                <xsl:when test="contains($A,'tOffset')"><xsl:text>@Cast(&quot;ni::tOffset&quot;) long</xsl:text></xsl:when>
                <xsl:when test="contains($A,'tSize')"><xsl:text>@Cast(&quot;ni::tSize&quot;) int</xsl:text></xsl:when>
                <xsl:when test="contains($A,'tBool')"><xsl:text>boolean</xsl:text></xsl:when>
                <xsl:when test="contains($A,'tInt')"><xsl:text>@Cast(&quot;ni::tInt&quot;) int</xsl:text></xsl:when>
                <xsl:when test="contains($A,'tUInt')"><xsl:text>@Cast(&quot;ni::tUInt&quot;) @Unsigned int</xsl:text></xsl:when>
                <xsl:otherwise>
                    <xsl:message>E/ Invalid Type : <xsl:value-of select="$A"/> in <xsl:value-of select="../../@name"/>::<xsl:value-of select="../@name"/></xsl:message>
                    <xsl:text>&lt;Invalid - Type&gt;</xsl:text>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:variable>

        <xsl:value-of select="concat($ANNO,$R)" />
    </xsl:template>

<!-- ************************************************************
* Get the Java Doc type markers (whether constant, passed by ptr, etc...)
************************************************************* -->
  <xsl:template name="get_doc_type_markers">
    <xsl:param name="A" />
        <xsl:variable name="R">
            <xsl:if test="&AISCONST;">Const </xsl:if>
            <xsl:if test="&AISPTRPTR;">PtrPtr </xsl:if>
            <xsl:if test="&AISPTR;">Ptr </xsl:if>
            <xsl:if test="&AISREF;">Ref </xsl:if>
        </xsl:variable>
        <xsl:value-of select="$R" />
  </xsl:template>

<!-- ************************************************************
* Get the Java Doc type
************************************************************* -->
    <xsl:template name="get_doc_type">
        <xsl:param name="A" />
        <xsl:param name="B" />

        <xsl:variable name="MARKERS">
            <xsl:if test="not(&AISCHARTYPE; or &AISINTERFACE; or &AISCOLLECTION; or contains($A,'cString'))">
                <xsl:call-template name="get_doc_type_markers">
                    <xsl:with-param name="A" select="$A" />
                </xsl:call-template>
            </xsl:if>
        </xsl:variable>

        <xsl:variable name="R">
            <xsl:choose>
                <xsl:when test="&AISPTRPTR;">
                    <xsl:text>PtrPtr</xsl:text>
                </xsl:when>
                <xsl:when test="&AISCOLLECTION;">
                    <xsl:text>iCollection(</xsl:text>
                    <xsl:call-template name="get_type_class"><xsl:with-param name="A"><xsl:value-of select="$A"/></xsl:with-param></xsl:call-template>
                    <xsl:text>)</xsl:text>
                </xsl:when>
                <xsl:when test="&AISINTERFACE;">
                    <xsl:call-template name="get_type_class"><xsl:with-param name="A"><xsl:value-of select="$A"/></xsl:with-param></xsl:call-template>
                </xsl:when>
                <xsl:when test="&AISENUM;">
                    <xsl:call-template name="get_type_class"><xsl:with-param name="A"><xsl:value-of select="$A"/></xsl:with-param></xsl:call-template>
                </xsl:when>
                <xsl:when test="&AISFLAGS;">
                    <xsl:call-template name="get_type_class"><xsl:with-param name="A"><xsl:value-of select="$A"/></xsl:with-param></xsl:call-template>
                </xsl:when>
                <!-- general cases -->
                <xsl:when test="contains($A,'sVec2i')"><xsl:text>Vec2i</xsl:text></xsl:when>
                <xsl:when test="contains($A,'sVec3i')"><xsl:text>Vec3i</xsl:text></xsl:when>
                <xsl:when test="contains($A,'sVec4i')"><xsl:text>Vec4i</xsl:text></xsl:when>
                <xsl:when test="contains($A,'sVec2f')"><xsl:text>Vec2f</xsl:text></xsl:when>
                <xsl:when test="contains($A,'sVec3f')"><xsl:text>Vec3f</xsl:text></xsl:when>
                <xsl:when test="contains($A,'sVec4f')"><xsl:text>Vec4f</xsl:text></xsl:when>
                <xsl:when test="contains($A,'sPlanef')"><xsl:text>Vec4f</xsl:text></xsl:when>
                <xsl:when test="contains($A,'sColor3f')"><xsl:text>Vec3f</xsl:text></xsl:when>
                <xsl:when test="contains($A,'sColor4f')"><xsl:text>Vec4f</xsl:text></xsl:when>
                <xsl:when test="contains($A,'sColor3ub')"><xsl:text>Color3ub</xsl:text></xsl:when>
                <xsl:when test="contains($A,'sColor4ub')"><xsl:text>Color4ub</xsl:text></xsl:when>
                <xsl:when test="contains($A,'sQuatf')"><xsl:text>Vec4f</xsl:text></xsl:when>
                <xsl:when test="contains($A,'sMatrixf')"><xsl:text>Matrixf</xsl:text></xsl:when>
                <xsl:when test="&AISPTR; and contains($A,'void')"><xsl:text>Ptr</xsl:text></xsl:when>
                <xsl:when test="starts-with($A,'void')"><xsl:text>void</xsl:text></xsl:when>
                <xsl:when test="&AISSTRUCT;">
                    <xsl:call-template name="get_type_class"><xsl:with-param name="A"><xsl:value-of select="$A"/></xsl:with-param></xsl:call-template>
                </xsl:when>
                <xsl:when test="contains($A,'sRecti')"><xsl:text>Vec4i</xsl:text></xsl:when>
                <xsl:when test="contains($A,'sRectf')"><xsl:text>Vec4f</xsl:text></xsl:when>
                <xsl:when test="&MATCH_VAR;"><xsl:text>Variant</xsl:text></xsl:when>
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
                <xsl:when test="contains($A,'cString')"><xsl:text>String(Native)</xsl:text></xsl:when>
                <xsl:when test="contains($A,'tBool')"><xsl:text>Boolean</xsl:text></xsl:when>
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
                <xsl:when test="contains($A,'tF32') or contains($A,'tFloat')"><xsl:text>F32</xsl:text></xsl:when>
                <xsl:when test="contains($A,'tF64')"><xsl:text>F64</xsl:text></xsl:when>
                <xsl:when test="contains($A,'achar') or contains($A,'cchar') or contains($A,' char') or contains($A,'uchar') or contains($A,'gchar') or contains($A,'xchar')">
                    <xsl:if test="not(starts-with($A,'const') and &AISPTR;)">
                        <xsl:message terminate='yes'>Char types can only be constant pointers.</xsl:message>
                    </xsl:if>
                    <xsl:choose>
                        <xsl:when test="contains($A,'achar')"><xsl:text>String(NativePtr)</xsl:text></xsl:when>
                        <xsl:when test="contains($A,'cchar') or contains($A,' char')"><xsl:text>String(UTF8Ptr)</xsl:text></xsl:when>
                        <xsl:when test="contains($A,'uchar')"><xsl:text>String(UnicodePtr)</xsl:text></xsl:when>
                        <xsl:when test="contains($A,'gchar')"><xsl:text>String(UTF16Ptr)</xsl:text></xsl:when>
                        <xsl:when test="contains($A,'xchar')"><xsl:text>String(UTF32Ptr)</xsl:text></xsl:when>
                    </xsl:choose>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:choose>
                        <xsl:when test="$B='yes'">
                            <xsl:text>Null</xsl:text>
                        </xsl:when>
                        <xsl:otherwise>
                            <xsl:text>Undef</xsl:text>
                        </xsl:otherwise>
                    </xsl:choose>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:variable>

        <xsl:value-of select="concat($MARKERS,$R)" />
    </xsl:template>

<!-- ************************************************************
* Get an enum value in Java
************************************************************* -->
  <xsl:template name="get_enum_value">
    <xsl:param name="VALUE" />
    <xsl:param name="ENUM" />
        <xsl:variable name="V0" select="replace(translate(replace($VALUE,&quot;&apos; &apos;&quot;,&quot;&apos;___&apos;&quot;),' ',''),&quot;&apos;___&apos;&quot;,&quot;&apos; &apos;&quot;)" />
        <xsl:variable name="V1" select="replace($V0,'niBit\(','ni.types.Flags.bit(')" />
        <xsl:variable name="V2" select="replace($V1,concat($ENUM,'_'),'')" />
        <!-- <xsl:variable name="V3" select="replace($V2,'eType_','EType.')" /> -->
        <!-- <xsl:variable name="V4" select="replace($V3,'eTypeFlags_','ETypeFlags.')" /> -->
        <xsl:variable name="V4" select="replace($V2,'niSecureHashType\(','TwoShort.build(')" />
        <xsl:variable name="V5" select="replace($V4,'eInvalidHandle','0xDEADBEEF')" />
        <xsl:variable name="V6" select="replace($V5,'niBuildColor\(','ULColor.build(')" />
        <xsl:variable name="V7" select="replace($V6,'niFourCC\(','FourCC.build(')" />
        <xsl:variable name="V8" select="replace($V7,'niMessageID\(','MessageID.build(')" />
        <xsl:variable name="V9" select="replace($V8,'e([A-Z][A-Za-z0-9]*)_([a-z0-9_]*)','E$1.$2')" />
        <xsl:variable name="V10" select="replace($V9,'E([A-Z][A-Za-z0-9]*)\.([0-9]+)([a-z0-9_]*)','E$1._$2$3')" />
        <xsl:variable name="V11" select="replace($V10,'VLK_ENTITY_MSGID\(([A-Z][A-Za-z0-9]*)',&quot;MessageID.build('_','K','E',EVlkEntityMessageType.$1&quot;)" />

        <xsl:value-of select="$V11"/>
  </xsl:template>

<!-- ************************************************************
* Get the jave header
************************************************************* -->
  <xsl:template name="get_java_header">
        <xsl:text>//</xsl:text><xsl:call-template name="newline"/>
        <xsl:text>// AUTO-GENERATED - DO NOT MODIFY</xsl:text><xsl:call-template name="newline"/>
        <xsl:text>//</xsl:text><xsl:call-template name="newline"/>

        <xsl:variable name="PKGNAME" select="/niIDL/@name" />
        <xsl:text>package ni.</xsl:text><xsl:value-of select="$PKGNAME"/><xsl:text>;</xsl:text><xsl:call-template name="newline"/>
        <xsl:call-template name="newline"/>

        <xsl:text>import java.nio.*;</xsl:text><xsl:call-template name="newline"/>
        <xsl:call-template name="newline"/>

        <xsl:text>import ni.types.*;</xsl:text><xsl:call-template name="newline"/>
        <xsl:text>import ni.types.annotations.*;</xsl:text><xsl:call-template name="newline"/>
        <xsl:call-template name="newline"/>

        <xsl:for-each select="/niIDL/dependencies/dependency">
            <xsl:variable name="DEP_PKGNAME" select="@name" />
            <xsl:text>import ni.</xsl:text><xsl:value-of select="$DEP_PKGNAME"/><xsl:text>.*;</xsl:text>
            <xsl:call-template name="newline"/>
        </xsl:for-each>

        <xsl:call-template name="newline"/>

    </xsl:template>

<!-- ************************************************************
* Get the jave header for Enum types
************************************************************* -->
  <xsl:template name="get_java_enum_header">
        <xsl:text>//</xsl:text><xsl:call-template name="newline"/>
        <xsl:text>// AUTO-GENERATED - DO NOT MODIFY</xsl:text><xsl:call-template name="newline"/>
        <xsl:text>//</xsl:text><xsl:call-template name="newline"/>

        <xsl:variable name="PKGNAME" select="/niIDL/@name" />
        <xsl:text>package ni.</xsl:text><xsl:value-of select="$PKGNAME"/><xsl:text>;</xsl:text><xsl:call-template name="newline"/>
        <xsl:text>import com.ts.core.*;</xsl:text><xsl:call-template name="newline"/>
    </xsl:template>

<!-- ************************************************************ -->
<!-- *** E O F ************************************************** -->
<!-- ************************************************************ -->
</xsl:transform>
