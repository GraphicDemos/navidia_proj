<?xml version="1.0" encoding="ISO-8859-1"?>
<xsl:stylesheet version="1.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method='html' version='1.0' encoding='UTF-8' indent='yes'/>

<xsl:template match="/">
  <html>
  <body>
  <h2>Build Results</h2>
    Total Errors: <xsl:value-of select="build/errors"/>
    <br/>
    Total Warnings: <xsl:value-of select="build/warnings"/>
    <br/>
    <br/>
    <table border="1">
      <tr bgcolor="#9acd32">
        <th align="left">Project</th>
        <th align="left">Errors</th>
        <th align="left">Warnings</th>
      </tr>
      <xsl:for-each select="build/project">
      <tr>
        <td><xsl:value-of select="title"/></td>
        <td><xsl:value-of select="errors"/></td>
        <td><xsl:value-of select="warnings"/></td>
      </tr>
      </xsl:for-each>
    </table>
  </body>
  </html>
</xsl:template>
</xsl:stylesheet>