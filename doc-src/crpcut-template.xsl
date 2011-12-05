<?xml version="1.0"?>
<xsl:stylesheet
   xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  <xsl:import href="CHUNK"/>
  <xsl:param name="chunk.first.sections"     select="1"/>
  <xsl:param name="navig.graphics"           select="1"/>
  <xsl:param name="navig.graphics.extension" select="'.png'"/>
  <xsl:param name="navig.graphics.path"      select="'images/'"/>
  <xsl:param name="admon.graphics"           select="1"/>
  <xsl:param name="admon.graphics.extension" select="'.png'"/>
  <xsl:param name="admon.graphics.path"      select="'images/'"/>
  <xsl:param name="admon.graphic.width"      select="57"/>
  <xsl:param name="html.stylesheet"          select="'crpcut-doc.css'"/>
  <xsl:param name="highlight.source"         select="1"/>
  <xsl:param name="use.id.as.filename"       select="1"/>
  <xsl:param name="use.extensions"           select="1"/>
  <xsl:param name="textinsert.extension"     select="1"/>
  <xsl:param name="toc.section.depth"        select="1"/>
  <xsl:param name="toc.max.depth"            select="2"/>
  <xsl:param name="callout.graphics"         select="0"/>

  <xsl:template name="user.head.content">
    <link href="https://plus.google.com/105064351736081696830" rel="publisher" />
  </xsl:template>

  <xsl:template match="version">
    <span>VERSION</span>
  </xsl:template>
  <xsl:template match="print_version">
    <span>PVERSION</span>
  </xsl:template>
  <xsl:template match="src_download_link">
    <a href="http://downloads.sourceforge.net/crpcut/crpcut-VERSION.tar.bz2">http://downloads.sourceforge.net/crpcut/crpcut-VERSION.tar.bz2</a>
  </xsl:template>
  <xsl:template match="doc_download_link">
    <a href="http://downloads.sourceforge.net/crpcut/crpcut-doc-VERSION.tar.bz2">http://downloads.sourceforge.net/crpcut/crpcut-doc-VERSION.tar.bz2</a>
  </xsl:template>
  <xsl:template name="user.header.navigation">
    <table width="100%">
      <tr><th class="slogan"  align="left">the <b>Compartmented Robust Posix C++ Unit Test</b> system</th><th></th>
<th align="right"><table><tr>
        <th class="sflogo" witdh="80" align="right">hosted by<br/><a href="http://sourceforge.net/projects/crpcut"><img src="http://sflogo.sourceforge.net/sflogo.php?group_id=251473&amp;type=10" width="80" height="15" alt="Get crpcut at SourceForge.net. Fast, secure and Free Open Source software downloads" /></a></th>
<th width="10"></th>
<th class="sflogo" align="left">support on<br/><a href="https://plus.google.com/105064351736081696830?prsrc=3" style="text-decoration:none;"><img src="https://ssl.gstatic.com/images/icons/gplus-16.png" alt="" style="border:0;width:16px;height:16px;"/></a></th>
</tr></table></th></tr>
    </table>
  </xsl:template>

</xsl:stylesheet>
