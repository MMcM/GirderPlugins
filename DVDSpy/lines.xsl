<?xml version="1.0"?>
<!--
Select 4 line or 2 line version of LCD sample script.
Commands / MultiGroups whose enabling should be changed are marked by Comments containing
*4L* or *2L*.
-->
<xsl:stylesheet version="1.0" 
		xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	        xmlns:UML="omg.org/UML/1.4">

  <xsl:param name="lines" select="2"/>

  <xsl:variable name="enable2">
    <xsl:choose>
      <xsl:when test="$lines &lt; 4">TRUE</xsl:when>
      <xsl:otherwise>FALSE</xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
  <xsl:variable name="enable4">
    <xsl:choose>
      <xsl:when test="$lines &gt; 2">TRUE</xsl:when>
      <xsl:otherwise>FALSE</xsl:otherwise>
    </xsl:choose>
  </xsl:variable>

  <xsl:template match="@Enabled">
    <xsl:attribute name="Enabled">
      <xsl:choose>
        <xsl:when test="../Comments/text()[contains(.,'*2L*')]">
          <xsl:value-of select="$enable2"/>
        </xsl:when>
        <xsl:when test="../Comments/text()[contains(.,'*4L*')]">
          <xsl:value-of select="$enable4"/>
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="."/>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:attribute>
  </xsl:template>

  <!-- Default copy templates -->
  <xsl:template match="@*|node()">
    <xsl:copy>
      <xsl:apply-templates select="@*|node()"/>
    </xsl:copy>
  </xsl:template>

</xsl:stylesheet>  
