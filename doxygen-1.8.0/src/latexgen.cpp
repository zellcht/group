/******************************************************************************
 *
 * $Id: latexgen.cpp,v 1.58 2001/03/19 19:27:40 root Exp $
 *
 * Copyright (C) 1997-2012 by Dimitri van Heesch.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation under the terms of the GNU General Public License is hereby 
 * granted. No representations are made about the suitability of this software 
 * for any purpose. It is provided "as is" without express or implied warranty.
 * See the GNU General Public License for more details.
 *
 * Documents produced by Doxygen are derivative works derived from the
 * input used in their production; they are not affected by this license.
 *
 */

#include <stdlib.h>

#include "qtbc.h"
#include <qdir.h>
#include "latexgen.h"
#include "config.h"
#include "message.h"
#include "doxygen.h"
#include "util.h"
#include "diagram.h"
#include "language.h"
#include "version.h"
#include "dot.h"
#include "pagedef.h"
#include "docparser.h"
#include "latexdocvisitor.h"
#include "dirdef.h"
#include "cite.h"

//static QCString filterTitle(const char *s)
//{
//  QCString tmp=s,result;
//  uint i;for (i=0;i<tmp.length();i++)
//  {
//    char c=tmp.at(i);
//    switch(c)
//    {
//      case '#': result+="\\#";  break;
//      case '"': result+="\\\""; break;
//      case '%': result+="\\%";  break;
//      case '[': result+="{";    break;
//      case ']': result+="}";    break;
//      default:  result+=c;      break;
//    }
//  }
//  return result;  
//}



LatexGenerator::LatexGenerator() : OutputGenerator()
{
  dir=Config_getString("LATEX_OUTPUT");
  col=0;
  //printf("LatexGenerator::LatexGenerator() insideTabbing=FALSE\n");
  insideTabbing=FALSE;
  firstDescItem=TRUE;
  disableLinks=FALSE;
  m_indent=0;
  templateMemberItem = FALSE;
  m_prettyCode=Config_getBool("LATEX_SOURCE_CODE");
}

LatexGenerator::~LatexGenerator()
{
}

static void writeLatexMakefile()
{
  bool generateBib = !Doxygen::citeDict->isEmpty();
  QCString dir=Config_getString("LATEX_OUTPUT");
  QCString fileName=dir+"/Makefile";
  QFile file(fileName);
  if (!file.open(IO_WriteOnly))
  {
    err("Could not open file %s for writing\n",fileName.data());
    exit(1);
  }
  // inserted by KONNO Akihisa <konno@researchers.jp> 2002-03-05
  QCString latex_command = Config_getString("LATEX_CMD_NAME");
  QCString mkidx_command = Config_getString("MAKEINDEX_CMD_NAME");
  // end insertion by KONNO Akihisa <konno@researchers.jp> 2002-03-05
  FTextStream t(&file);
  if (!Config_getBool("USE_PDFLATEX")) // use plain old latex
  {
    t << "all: refman.dvi" << endl
      << endl
      << "ps: refman.ps" << endl
      << endl
      << "pdf: refman.pdf" << endl
      << endl
      << "ps_2on1: refman_2on1.ps" << endl
      << endl
      << "pdf_2on1: refman_2on1.pdf" << endl
      << endl
      << "refman.ps: refman.dvi" << endl
      << "\tdvips -o refman.ps refman.dvi" << endl
      << endl;
    t << "refman.pdf: refman.ps" << endl;
    t << "\tps2pdf refman.ps refman.pdf" << endl << endl;
    t << "refman.dvi: clean refman.tex doxygen.sty" << endl
      << "\techo \"Running latex...\"" << endl
      << "\t" << latex_command << " refman.tex" << endl
      << "\techo \"Running makeindex...\"" << endl
      << "\t" << mkidx_command << " refman.idx" << endl;
    if (generateBib)
    {
      t << "\techo \"Running bibtex...\"" << endl;
      t << "\tbibtex refman" << endl;
      t << "\techo \"Rerunning latex....\"" << endl;
      t << "\t" << latex_command << " refman.tex" << endl;
    }
    t << "\techo \"Rerunning latex....\"" << endl
      << "\t" << latex_command << " refman.tex" << endl
      << "\tlatex_count=5 ; \\" << endl
      << "\twhile egrep -s 'Rerun (LaTeX|to get cross-references right)' refman.log && [ $$latex_count -gt 0 ] ;\\" << endl
      << "\t    do \\" << endl
      << "\t      echo \"Rerunning latex....\" ;\\" << endl
      << "\t      " << latex_command << " refman.tex ;\\" << endl
      << "\t      latex_count=`expr $$latex_count - 1` ;\\" << endl
      << "\t    done" << endl << endl
      << "refman_2on1.ps: refman.ps" << endl
      << "\tpsnup -2 refman.ps >refman_2on1.ps" << endl
      << endl
      << "refman_2on1.pdf: refman_2on1.ps" << endl
      << "\tps2pdf refman_2on1.ps refman_2on1.pdf" << endl;
  }
  else // use pdflatex for higher quality output
  {
    t << "all: refman.pdf" << endl << endl
      << "pdf: refman.pdf" << endl << endl;
    t << "refman.pdf: clean refman.tex" << endl;
    t << "\tpdflatex refman" << endl;
    t << "\t" << mkidx_command << " refman.idx" << endl;
    if (generateBib)
    {
      t << "\tbibtex refman" << endl;
      t << "\tpdflatex refman" << endl;
    }
    t << "\tpdflatex refman" << endl
      << "\tlatex_count=5 ; \\" << endl
      << "\twhile egrep -s 'Rerun (LaTeX|to get cross-references right)' refman.log && [ $$latex_count -gt 0 ] ;\\" << endl
      << "\t    do \\" << endl
      << "\t      echo \"Rerunning latex....\" ;\\" << endl
      << "\t      pdflatex refman ;\\" << endl
      << "\t      latex_count=`expr $$latex_count - 1` ;\\" << endl
      << "\t    done" << endl << endl;
  }

  t << endl
    << "clean:" << endl
    << "\trm -f " 
    << "*.ps *.dvi *.aux *.toc *.idx *.ind *.ilg *.log *.out *.brf *.blg *.bbl refman.pdf" << endl;
}

static void writeMakeBat()
{
#if defined(_MSC_VER)
  QCString dir=Config_getString("LATEX_OUTPUT");
  QCString fileName=dir+"/make.bat";
  QCString latex_command = Config_getString("LATEX_CMD_NAME");
  QCString mkidx_command = Config_getString("MAKEINDEX_CMD_NAME");
  QFile file(fileName);
  bool generateBib = !Doxygen::citeDict->isEmpty();
  if (!file.open(IO_WriteOnly))
  {
    err("Could not open file %s for writing\n",fileName.data());
    exit(1);
  }
  FTextStream t(&file);
  t << "del /s /f *.ps *.dvi *.aux *.toc *.idx *.ind *.ilg *.log *.out *.brf *.blg *.bbl refman.pdf\n\n";
  if (!Config_getBool("USE_PDFLATEX")) // use plain old latex
  {
    t << latex_command << " refman.tex\n";
    t << "echo ----\n";
    t << mkidx_command << " refman.idx\n";
    if (generateBib)
    {
      t << "bibtex refman\n";
      t << "echo ----\n";
      t << latex_command << " refman.tex\n";
    }
    t << "setlocal enabledelayedexpansion\n";
    t << "set count=5\n";
    t << ":repeat\n";
    t << "set content=X\n";
    t << "for /F \"tokens=*\" %%T in ( 'findstr /C:\"Rerun LaTeX\" refman.log' ) do set content=\"%%~T\"\n";
    t << "if !content! == X for /F \"tokens=*\" %%T in ( 'findstr /C:\"Rerun to get cross-references right\" refman.log' ) do set content=\"%%~T\"\n";
    t << "if !content! == X goto :skip\n";
    t << "set /a count-=1\n";
    t << "if !count! EQU 0 goto :skip\n\n";
    t << "echo ----\n";
    t << latex_command << " refman.tex\n";
    t << "goto :repeat\n";
    t << ":skip\n";
    t << "endlocal\n";
    t << "dvips -o refman.ps refman.dvi\n";
    t << "gswin32c -q -dNOPAUSE -dBATCH -sDEVICE=pdfwrite "
         "-sOutputFile=refman.pdf -c save pop -f refman.ps\n";
  }
  else // use pdflatex
  {
    t << "pdflatex refman\n";
    t << "echo ----\n";
    t << mkidx_command << " refman.idx\n";
    if (generateBib)
    {
      t << "bibtex refman" << endl;
      t << "pdflatex refman" << endl;
    }
    t << "echo ----\n";
    t << "pdflatex refman\n\n";
    t << "setlocal enabledelayedexpansion\n";
    t << "set count=5\n";
    t << ":repeat\n";
    t << "set content=X\n";
    t << "for /F \"tokens=*\" %%T in ( 'findstr /C:\"Rerun LaTeX\" refman.log' ) do set content=\"%%~T\"\n";
    t << "if !content! == X for /F \"tokens=*\" %%T in ( 'findstr /C:\"Rerun to get cross-references right\" refman.log' ) do set content=\"%%~T\"\n";
    t << "if !content! == X goto :skip\n";
    t << "set /a count-=1\n";
    t << "if !count! EQU 0 goto :skip\n\n";
    t << "echo ----\n";
    t << "pdflatex refman\n";
    t << "goto :repeat\n";
    t << ":skip\n";
    t << "endlocal\n";
  }
#endif
}

void LatexGenerator::init()
{

  QCString dir=Config_getString("LATEX_OUTPUT");
  QDir d(dir);
  if (!d.exists() && !d.mkdir(dir))
  {
    err("Could not create output directory %s\n",dir.data());
    exit(1);
  }

  writeLatexMakefile();
  writeMakeBat();

  createSubDirs(d);
}

static void writeDefaultHeaderPart1(FTextStream &t)
{
  // part 1

  QCString paperName;
  if (Config_getBool("LATEX_BATCHMODE")) t << "\\batchmode" << endl;
  QCString &paperType=Config_getEnum("PAPER_TYPE");
  if (paperType=="a4wide") 
    paperName="a4"; 
  else 
    paperName=paperType;
  t << "\\documentclass";
  //"[" << paperName << "paper";
  //t << "]";
  t << "{";
  if (Config_getBool("COMPACT_LATEX")) t << "article"; else t << "book";
  t << "}\n";
  // the next package is obsolete (see bug 563698)
  //if (paperType=="a4wide") t << "\\usepackage{a4wide}\n";
  t << 
    "\\usepackage["<<paperName<<"paper,top=2.5cm,bottom=2.5cm,left=2.5cm,right=2.5cm]{geometry}\n"
    "\\usepackage{makeidx}\n"
    "\\usepackage{natbib}\n"
    "\\usepackage{graphicx}\n"
    "\\usepackage{multicol}\n"
    "\\usepackage{float}\n"
    "\\usepackage{listings}\n"
    "\\usepackage{color}\n"
    "\\usepackage{ifthen}\n"
    "\\usepackage[table]{xcolor}\n"
    "\\usepackage{textcomp}\n"
    "\\usepackage{alltt}\n"
    //"\\usepackage{ae,aecompl,aeguill}\n"
    ;
  //if (Config_getBool("USE_PDFLATEX"))
  //{
  //  t << "\\usepackage{times}" << endl;
  //}
  if (Config_getBool("PDF_HYPERLINKS")) 
  {
    t << "\\usepackage{ifpdf}" << endl
      << "\\ifpdf" << endl
      << "\\usepackage[pdftex," << endl
      << "            pagebackref=true," << endl
      << "            colorlinks=true," << endl
      << "            linkcolor=blue," << endl
      << "            unicode" << endl
      << "           ]{hyperref}" << endl
      << "\\else" << endl
      << "\\usepackage[ps2pdf," << endl
      << "            pagebackref=true," << endl
      << "            colorlinks=true," << endl
      << "            linkcolor=blue," << endl
      << "            unicode" << endl
      << "           ]{hyperref}" << endl
      << "\\usepackage{pspicture}" << endl
      << "\\fi" << endl;
  }
  // Try to get the command for switching on the language
  // support
  t << "\\usepackage[utf8]{inputenc}" << endl;
  QCString sLanguageSupportCommand(
      theTranslator->latexLanguageSupportCommand());

  if (!sLanguageSupportCommand.isEmpty())
  {
    // The command is not empty.  Put it to the output.
    // if the command is empty, no output is needed.
    t << sLanguageSupportCommand << endl;
  }
  t << "\\usepackage{mathptmx}\n";
  t << "\\usepackage[scaled=.90]{helvet}\n";
  t << "\\usepackage{courier}\n";
  t << "\\usepackage{sectsty}\n";
  t << "\\usepackage[titles]{tocloft}\n";
  t << "\\usepackage{doxygen}\n";

  // define option for listings
  t << "\\lstset{language=C++,"
                "inputencoding=utf8,"
                "basicstyle=\\footnotesize,"
                "breaklines=true,"
                "breakatwhitespace=true,"
                "tabsize=" << Config_getInt("TAB_SIZE") <<","
                "numbers=left }" << endl;

  QStrList &extraPackages = Config_getList("EXTRA_PACKAGES");
  const char *s=extraPackages.first();
  while (s)
  {
    t << "\\usepackage{" << s << "}\n";
    s=extraPackages.next();
  }
  t << "\\makeindex\n"
    "\\setcounter{tocdepth}{3}\n"
    "\\renewcommand{\\footrulewidth}{0.4pt}\n"
    "\\renewcommand{\\familydefault}{\\sfdefault}\n"
    "\\hfuzz=15pt\n"  // allow a bit of overflow to go unnoticed
    "\\setlength{\\emergencystretch}{15pt}\n"
    "\\hbadness=750\n"
    "\\tolerance=750\n"
    "\\begin{document}\n";
  static bool pdfHyperlinks = Config_getBool("PDF_HYPERLINKS");
  static bool usePDFLatex   = Config_getBool("USE_PDFLATEX");
  if (pdfHyperlinks && usePDFLatex)
  {
    // to avoid duplicate page anchors due to reuse of same numbers for
    // the index (be it as roman numbers)
    t << "\\hypersetup{pageanchor=false,citecolor=blue}" << endl;
  }
  if (theTranslator->idLanguage()=="greek") t << "\\selectlanguage{greek}\n";
  t << "\\begin{titlepage}\n"
    "\\vspace*{7cm}\n"
    "\\begin{center}\n"
    "{\\Large ";

}

static void writeDefaultHeaderPart2(FTextStream &t)
{
  // part 2
  t << "}\\\\" << endl
    << "\\vspace*{1cm}" << endl
    << "{\\large ";
}

static void writeDefaultHeaderPart3(FTextStream &t)
{
  // part 3
  t << " Doxygen " << versionString << "}\\\\" << endl
    << "\\vspace*{0.5cm}" << endl
    << "{\\small " << dateToString(TRUE) << "}\\\\" << endl
    << "\\end{center}" << endl
    << "\\end{titlepage}" << endl;
  if (!Config_getBool("COMPACT_LATEX")) t << "\\clearemptydoublepage\n";
  t << "\\pagenumbering{roman}\n";
  t << "\\tableofcontents\n";
  if (!Config_getBool("COMPACT_LATEX")) t << "\\clearemptydoublepage\n";
  t << "\\pagenumbering{arabic}\n";
  static bool pdfHyperlinks = Config_getBool("PDF_HYPERLINKS");
  static bool usePDFLatex   = Config_getBool("USE_PDFLATEX");
  if (pdfHyperlinks && usePDFLatex)
  {
    t << "\\hypersetup{pageanchor=true,citecolor=blue}" << endl;
  }
}

static void writeDefaultStyleSheetPart1(FTextStream &t)
{
  // part 1
  t << "\\NeedsTeXFormat{LaTeX2e}\n"
       "\\ProvidesPackage{doxygen}\n\n";
  t << "% Packages used by this style file\n"
       "\\RequirePackage{alltt}\n"
       "\\RequirePackage{array}\n"
       "\\RequirePackage{calc}\n"
       "\\RequirePackage{color}\n"
       "\\RequirePackage{fancyhdr}\n"
       "\\RequirePackage{longtable}\n"
       "\\RequirePackage{verbatim}\n"
       "\\RequirePackage{ifthen}\n"
       "\\RequirePackage{xtab}\n"
       "\\RequirePackage[table]{xcolor}\n\n";

  t << "% Use helvetica font instead of times roman\n"
       "\\RequirePackage{helvet}\n"
       "\\RequirePackage{sectsty}\n"
       "\\RequirePackage{tocloft}\n"
//       "\\allsectionsfont{\\usefont{OT1}{phv}{bc}{n}\\selectfont}\n"
//       "\\providecommand{\\cftchapfont}{%\n"
//       "  \\fontsize{11}{13}\\usefont{OT1}{phv}{bc}{n}\\selectfont\n"
//       "}\n"
//       "\\providecommand{\\cftchappagefont}{%\n"
//       "  \\fontsize{11}{13}\\usefont{OT1}{phv}{c}{n}\\selectfont\n"
//       "}\n"
//       "\\providecommand{\\cftsecfont}{%\n"
//       "  \\fontsize{10}{12}\\usefont{OT1}{phv}{c}{n}\\selectfont\n"
//       "}\n"
//       "\\providecommand{\\cftsecpagefont}{%\n"
//       "  \\fontsize{10}{12}\\usefont{OT1}{phv}{c}{n}\\selectfont\n"
//       "}\n"
//       "\\providecommand{\\cftsubsecfont}{%\n"
//       "  \\fontsize{10}{12}\\usefont{OT1}{phv}{c}{n}\\selectfont\n"
//       "}\n"
//       "\\providecommand{\\cftsubsecpagefont}{%\n"
//       "  \\fontsize{10}{12}\\usefont{OT1}{phv}{c}{n}\\selectfont\n"
//       "}\n"
//       "\\providecommand{\\cftsubsubsecfont}{%\n"
//       "  \\fontsize{9}{11}\\usefont{OT1}{phv}{c}{n}\\selectfont\n"
//       "}\n"
//       "\\providecommand{\\cftsubsubsecpagefont}{%\n"
//       "  \\fontsize{9}{11}\\usefont{OT1}{phv}{c}{n}\\selectfont\n"
//       "}\n"
//       "\\providecommand{\\cftparafont}{%\n"
//       "  \\fontsize{9}{11}\\usefont{OT1}{phv}{c}{n}\\selectfont\n"
//       "}\n"
//       "\\providecommand{\\cftparapagefont}{%\n"
//       "  \\fontsize{9}{11}\\usefont{OT1}{phv}{c}{n}\\selectfont\n"
//       "}\n"
//       "\\providecommand{\\cfttoctitlefont}{%\n"
//       "  \\fontsize{20}{22}\\usefont{OT1}{phv}{b}{n}\\selectfont\n"
//       "}\n"
       "\\providecommand{\\rmdefault}{phv}\n"
       "\\providecommand{\\bfdefault}{bc}\n"
       "\n\n";

  t << "% Setup fancy headings\n"
       "\\pagestyle{fancyplain}\n"
       "\\newcommand{\\clearemptydoublepage}{%\n"
       "  \\newpage{\\pagestyle{empty}\\cleardoublepage}%\n"
       "}\n";
  if (!Config_getBool("COMPACT_LATEX")) 
    t << "\\renewcommand{\\chaptermark}[1]{%\n"
         "  \\markboth{#1}{}%\n"
         "}\n";
  t << "\\renewcommand{\\sectionmark}[1]{%\n"
       "  \\markright{\\thesection\\ #1}%\n"
       "}\n";

  //t << "\\lhead[\\fancyplain{}{\\bfseries\\thepage}]{%\n"
  //     "  \\fancyplain{}{\\bfseries\\rightmark}%\n"
  //     "}\n";
  //t << "\\rhead[\\fancyplain{}{\\bfseries\\leftmark}]{%\n"
  //     "  \\fancyplain{}{\\bfseries\\thepage}%\n"
  //     "}\n";
  //t << "\\rfoot[\\fancyplain{}{\\bfseries\\scriptsize%\n  ";
  t << "\\fancyhead[LE]{\\fancyplain{}{\\bfseries\\thepage}}\n";
  t << "\\fancyhead[CE]{\\fancyplain{}{}}\n";
  t << "\\fancyhead[RE]{\\fancyplain{}{\\bfseries\\leftmark}}\n";
  t << "\\fancyhead[LO]{\\fancyplain{}{\\bfseries\\rightmark}}\n";
  t << "\\fancyhead[CO]{\\fancyplain{}{}}\n";
  t << "\\fancyhead[RO]{\\fancyplain{}{\\bfseries\\thepage}}\n";

  t << "\\fancyfoot[LE]{\\fancyplain{}{}}\n";
  t << "\\fancyfoot[CE]{\\fancyplain{}{}}\n";
  t << "\\fancyfoot[RE]{\\fancyplain{}{\\bfseries\\scriptsize ";
}

static void writeDefaultStyleSheetPart2(FTextStream &t)
{
  t << "}}\n";
  t << "\\fancyfoot[LO]{\\fancyplain{}{\\bfseries\\scriptsize ";
  //t << "\\lfoot[]{\\fancyplain{}{\\bfseries\\scriptsize%\n  ";

}

static void writeDefaultStyleSheetPart3(FTextStream &t)
{
  static bool latexSourceCode = Config_getBool("LATEX_SOURCE_CODE");
  t << "}}\n";
  //t << "\\cfoot{}\n\n";
  t << "\\fancyfoot[CO]{\\fancyplain{}{}}\n";
  t << "\\fancyfoot[RO]{\\fancyplain{}{}}\n";

  t << "%---------- Internal commands used in this style file ----------------\n\n";

  t << "\\newcommand\\tabfill[1]{%\n";
  t << "  \\dimen@\\linewidth%\n";
  t << "  \\advance\\dimen@\\@totalleftmargin%\n";
  t << "  \\advance\\dimen@-\\dimen\\@curtab%\n";
  t << "  \\parbox[t]\\dimen@{\\raggedright #1\\ifhmode\\strut\\fi}%\n";
  t << "}\n\n";

  t << "\\newcommand{\\ensurespace}[1]{%\n";
  t << "  \\begingroup\n";
  t << "    \\setlength{\\dimen@}{#1}%\n";
  t << "    \\vskip\\z@\\@plus\\dimen@\n";
  t << "    \\penalty -100\\vskip\\z@\\@plus -\\dimen@\n";
  t << "    \\vskip\\dimen@\n";
  t << "    \\penalty 9999%\n";
  t << "    \\vskip -\\dimen@\n";
  t << "    \\vskip\\z@skip % hide the previous |\\vskip| from |\\addvspace|\n";
  t << "  \\endgroup\n";
  t << "}\n\n";

  t << "% Generic environment used by all paragraph-based environments defined\n"
       "% below. Note that the command \\title{...} needs to be defined inside\n"
       "% those environments!\n"
       "\\newenvironment{DoxyDesc}[1]{%\n"
       //"  \\filbreak%\n"
       "  \\ensurespace{4\\baselineskip}%\n"
       "  \\begin{list}{}%\n"
       "  {%\n"
       "    \\settowidth{\\labelwidth}{40pt}%\n"
       "    \\setlength{\\leftmargin}{\\labelwidth}%\n"
       "    \\setlength{\\parsep}{0pt}%\n"
       "    \\setlength{\\itemsep}{-4pt}%\n"
       "    \\renewcommand{\\makelabel}{\\entrylabel}%\n"
       "  }%\n"
       "  \\item[#1]%\n"
       "}{%\n"
       "  \\end{list}%\n"
       "}\n\n";
  t << "%---------- Commands used by doxygen LaTeX output generator ----------\n\n";
  t << "% Used by <pre> ... </pre>\n"
       "\\newenvironment{DoxyPre}{%\n"
       "  \\small%\n"
       "  \\begin{alltt}%\n"
       "}{%\n"
       "  \\end{alltt}%\n"
       "  \\normalsize%\n"
       "}\n\n";
  t << "% Used by @code ... @endcode\n"
       "\\newenvironment{DoxyCode}{%\n";
  if (latexSourceCode)
  {
    t << "\n\n\\begin{scriptsize}\\begin{alltt}%" << endl;
  }
  else
  {
    t << "  \\footnotesize%\n"
         "  \\verbatim%\n";
  }
  t << "}{%\n";
  if (latexSourceCode)
  {
    t << "\\end{alltt}\\end{scriptsize}%" << endl; 
  }
  else
  {
    t << "  \\endverbatim%\n"
         "  \\normalsize%\n";
  }
  t << "}\n\n";
  t << "% Used by @example, @include, @includelineno and @dontinclude\n"
       "\\newenvironment{DoxyCodeInclude}{%\n"
       "  \\DoxyCode%\n"
       "}{%\n"
       "  \\endDoxyCode%\n"
       "}\n\n";
  t << "% Used by @verbatim ... @endverbatim\n"
       "\\newenvironment{DoxyVerb}{%\n"
       "  \\footnotesize%\n"
       "  \\verbatim%\n"
       "}{%\n"
       "  \\endverbatim%\n"
       "  \\normalsize%\n"
       "}\n\n";
  t << "% Used by @verbinclude\n"
       "\\newenvironment{DoxyVerbInclude}{%\n"
       "  \\DoxyVerb%\n"
       "}{%\n"
       "  \\endDoxyVerb%\n"
       "}\n\n";
  t << "% Used by numbered lists (using '-#' or <ol> ... </ol>)\n"
       "\\newenvironment{DoxyEnumerate}{%\n"
       "  \\enumerate%\n"
       "}{%\n"
       "  \\endenumerate%\n"
       "}\n\n";
  t << "% Used by bullet lists (using '-', @li, @arg, or <ul> ... </ul>)\n"
       "\\newenvironment{DoxyItemize}{%\n"
       "  \\itemize%\n"
       "}{%\n"
       "  \\enditemize%\n"
       "}\n\n";
  t << "% Used by description lists (using <dl> ... </dl>)\n"
       "\\newenvironment{DoxyDescription}{%\n"
       "  \\description%\n"
       "}{%\n"
       "  \\enddescription%\n"
       "}\n\n";
  t << "% Used by @image, @dotfile, and @dot ... @enddot\n"
       "% (only if caption is specified)\n"
       "\\newenvironment{DoxyImage}{%\n"
       "  \\begin{figure}[H]%\n"
       "  \\begin{center}%\n"
       "}{%\n"
       "  \\end{center}%\n"
       "  \\end{figure}%\n"
       "}\n\n";
  t << "% Used by @image, @dotfile, @dot ... @enddot, and @msc ... @endmsc\n"
       "% (only if no caption is specified)\n"
       "\\newenvironment{DoxyImageNoCaption}{%\n"
       "}{%\n"
       "}\n\n";
  t << "% Used by @attention\n"
       "\\newenvironment{DoxyAttention}[1]{%\n"
       "  \\begin{DoxyDesc}{#1}%\n"
       "}{%\n"
       "  \\end{DoxyDesc}%\n"
       "}\n\n";
  t << "% Used by @author and @authors\n"
       "\\newenvironment{DoxyAuthor}[1]{%\n"
       "  \\begin{DoxyDesc}{#1}%\n"
       "}{%\n"
       "  \\end{DoxyDesc}%\n"
       "}\n\n";
  t << "% Used by @date\n"
       "\\newenvironment{DoxyDate}[1]{%\n"
       "  \\begin{DoxyDesc}{#1}%\n"
       "}{%\n"
       "  \\end{DoxyDesc}%\n"
       "}\n\n";
  t << "% Used by @invariant\n"
       "\\newenvironment{DoxyInvariant}[1]{%\n"
       "  \\begin{DoxyDesc}{#1}%\n"
       "}{%\n"
       "  \\end{DoxyDesc}%\n"
       "}\n\n";
  t << "% Used by @note\n"
       "\\newenvironment{DoxyNote}[1]{%\n"
       "  \\begin{DoxyDesc}{#1}%\n"
       "}{%\n"
       "  \\end{DoxyDesc}%\n"
       "}\n\n";
  t << "% Used by @post\n"
       "\\newenvironment{DoxyPostcond}[1]{%\n"
       "  \\begin{DoxyDesc}{#1}%\n"
       "}{%\n"
       "  \\end{DoxyDesc}%\n"
       "}\n\n";
  t << "% Used by @pre\n"
       "\\newenvironment{DoxyPrecond}[1]{%\n"
       "  \\begin{DoxyDesc}{#1}%\n"
       "}{%\n"
       "  \\end{DoxyDesc}%\n"
       "}\n\n";
  t << "% Used by @copyright\n"
       "\\newenvironment{DoxyCopyright}[1]{%\n"
       "  \\begin{DoxyDesc}{#1}%\n"
       "}{%\n"
       "  \\end{DoxyDesc}%\n"
       "}\n\n";
  t << "% Used by @remark\n"
       "\\newenvironment{DoxyRemark}[1]{%\n"
       "  \\begin{DoxyDesc}{#1}%\n"
       "}{%\n"
       "  \\end{DoxyDesc}%\n"
       "}\n\n";
  t << "% Used by @return\n"
       "\\newenvironment{DoxyReturn}[1]{%\n"
       "  \\begin{DoxyDesc}{#1}%\n"
       "}{%\n"
       "  \\end{DoxyDesc}%\n"
       "}\n\n";
  t << "% Used by @since\n"
       "\\newenvironment{DoxySince}[1]{%\n"
       "  \\begin{DoxyDesc}{#1}%\n"
       "}{%\n"
       "  \\end{DoxyDesc}%\n"
       "}\n\n";
  t << "% Used by @see\n"
       "\\newenvironment{DoxySeeAlso}[1]{%\n"
       "  \\begin{DoxyDesc}{#1}%\n"
       "}{%\n"
       "  \\end{DoxyDesc}%\n"
       "}\n\n";
  t << "% Used by @version\n"
       "\\newenvironment{DoxyVersion}[1]{%\n"
       "  \\begin{DoxyDesc}{#1}%\n"
       "}{%\n"
       "  \\end{DoxyDesc}%\n"
       "}\n\n";
  t << "% Used by @warning\n"
       "\\newenvironment{DoxyWarning}[1]{%\n"
       "  \\begin{DoxyDesc}{#1}%\n"
       "}{%\n"
       "  \\end{DoxyDesc}%\n"
       "}\n\n";
  t << "% Used by @internal\n"
       "\\newenvironment{DoxyInternal}[1]{%\n"
       "  \\paragraph*{#1}%\n"
       "}{%\n"
       "}\n\n";
  t << "% Used by @par and @paragraph\n"
       "\\newenvironment{DoxyParagraph}[1]{%\n"
       "  \\begin{list}{}%\n"
       "  {%\n"
       "    \\settowidth{\\labelwidth}{40pt}%\n"
       "    \\setlength{\\leftmargin}{\\labelwidth}%\n"
       "    \\setlength{\\parsep}{0pt}%\n"
       "    \\setlength{\\itemsep}{-4pt}%\n"
       "    \\renewcommand{\\makelabel}{\\entrylabel}%\n"
       "  }%\n"
       "  \\item[#1]%\n"
       "}{%\n"
       "  \\end{list}%\n"
       "}\n\n";
  t << "% Used by parameter lists\n"
       "\\newenvironment{DoxyParams}[2][]{%\n"
       "  \\begin{DoxyDesc}{#2}%\n"
       //"    \\begin{description}%\n"
       "    \\item[] \\hspace{\\fill} \\vspace{-40pt}%\n"
       //"      \\definecolor{tableShade}{HTML}{F8F8F8}%\n"
       //"      \\rowcolors{1}{white}{tableShade}%\n"
       //"      \\arrayrulecolor{gray}%\n"
       "    \\settowidth{\\labelwidth}{40pt}%\n"
       //"    \\setlength{\\LTleft}{\\labelwidth}%\n"
       "    \\setlength{\\LTleft}{0pt}%\n"
       "    \\setlength{\\tabcolsep}{0.01\\textwidth}%\n"
       "    \\ifthenelse{\\equal{#1}{}}%\n" // default: name, docs columns
       "    {\\begin{longtable}{|>{\\raggedleft\\hspace{0pt}}p{0.15\\textwidth}|%\n"
       "                        p{0.815\\textwidth}|}}%\n"
       "    {\\ifthenelse{\\equal{#1}{1}}%\n" // inout, name, docs columns, or type, name, docs columns
       "      {\\begin{longtable}{|>{\\centering}p{0.10\\textwidth}|%\n"
       "                         >{\\raggedleft\\hspace{0pt}}p{0.15\\textwidth}|%\n"
       "                         p{0.685\\textwidth}|}}%\n"
       "      {\\begin{longtable}{|>{\\centering}p{0.10\\textwidth}|%\n" // inout, type, name, docs columns
       "                         >{\\centering\\hspace{0pt}}p{0.15\\textwidth}|%\n"
       "                         >{\\raggedleft\\hspace{0pt}}p{0.15\\textwidth}|%\n"
       "                         p{0.515\\textwidth}|}}%\n"
       "    }\\hline%\n"
       "}{%\n"
       "    \\end{longtable}%\n"
       //"    \\end{description}%\n"
       "  \\end{DoxyDesc}%\n"
       "}\n\n";
  t << "% Used for fields of simple structs\n"
       "\\newenvironment{DoxyFields}[1]{%\n"
       "  \\begin{DoxyDesc}{#1}%\n"
       "    \\item[] \\hspace{\\fill} \\vspace{-40pt}%\n"
       "    \\settowidth{\\labelwidth}{40pt}%\n"
       "    \\setlength{\\LTleft}{0pt}%\n"
       "    \\setlength{\\tabcolsep}{0.01\\textwidth}%\n"
       "    \\begin{longtable}{|>{\\raggedleft\\hspace{0pt}}p{0.15\\textwidth}|%\n"
       "                         p{0.15\\textwidth}|%\n"
       "                         p{0.635\\textwidth}|}%\n"
       //"\\hline{\\sf\\textbf{Type}} & {\\sf\\textbf{Name}} & {\\sf\\textbf{Description}}\\endhead%\n"
       "    \\hline%\n"
       "}{%\n"
       "    \\end{longtable}%\n"
       //"    \\end{description}%\n"
       "  \\end{DoxyDesc}%\n"
       "}\n\n";
  t << "% is used for parameters within a detailed function description\n"
       "\\newenvironment{DoxyParamCaption}{%\n"
       "  \\renewcommand{\\item}[2][]{##1 {\\em ##2}}%\n"
       "  }{%\n"
       "}\n\n";
  t << "% Used by return value lists\n"
       "\\newenvironment{DoxyRetVals}[1]{%\n"
       "  \\begin{DoxyDesc}{#1}%\n"
       "    \\begin{description}%\n"
       "      \\item[] \\hspace{\\fill} \\vspace{-25pt}%\n"
       //"      \\definecolor{tableShade}{HTML}{F8F8F8}%\n"
       //"      \\rowcolors{1}{white}{tableShade}%\n"
       //"      \\arrayrulecolor{gray}%\n"
       "      \\setlength{\\tabcolsep}{0.01\\textwidth}%\n"
       "      \\begin{longtable}{|>{\\raggedleft\\hspace{0pt}}p{0.25\\textwidth}|%\n"
       "                          p{0.77\\textwidth}|}%\n"
       "      \\hline%\n"
       "}{%\n"
       "      \\end{longtable}%\n"
       "    \\end{description}%\n"
       "  \\end{DoxyDesc}%\n"
       "}\n\n";
  t << "% Used by exception lists\n"
       "\\newenvironment{DoxyExceptions}[1]{%\n"
       "  \\begin{DoxyDesc}{#1}%\n"
       "    \\begin{description}%\n"
       "      \\item[] \\hspace{\\fill} \\vspace{-25pt}%\n"
       "      \\definecolor{tableShade}{HTML}{F8F8F8}%\n"
       "      \\rowcolors{1}{white}{tableShade}%\n"
       "      \\arrayrulecolor{gray}%\n"
       "      \\setlength{\\tabcolsep}{0.01\\textwidth}%\n"
       "      \\begin{longtable}{|>{\\raggedleft\\hspace{0pt}}p{0.25\\textwidth}|%\n"
       "                          p{0.77\\textwidth}|}%\n"
       "      \\hline%\n"
       "}{%\n"
       "      \\end{longtable}%\n"
       "    \\end{description}%\n"
       "  \\end{DoxyDesc}%\n"
       "}\n\n";
  t << "% Used by template parameter lists\n"
       "\\newenvironment{DoxyTemplParams}[1]{%\n"
       "  \\begin{DoxyDesc}{#1}%\n"
       "    \\begin{description}%\n"
       "      \\item[] \\hspace{\\fill} \\vspace{-25pt}%\n"
       "      \\definecolor{tableShade}{HTML}{F8F8F8}%\n"
       "      \\rowcolors{1}{white}{tableShade}%\n"
       "      \\arrayrulecolor{gray}%\n"
       "      \\setlength{\\tabcolsep}{0.01\\textwidth}%\n"
       "      \\begin{longtable}{|>{\\raggedleft\\hspace{0pt}}p{0.25\\textwidth}|%\n"
       "                          p{0.77\\textwidth}|}%\n"
       "      \\hline%\n"
       "}{%\n"
       "      \\end{longtable}%\n"
       "    \\end{description}%\n"
       "  \\end{DoxyDesc}%\n"
       "}\n\n";
  t << "\\newcommand{\\doxyref}[3]{\\textbf{#1} (\\textnormal{#2}\\,\\pageref{#3})}\n";
  t << "\\newenvironment{DoxyCompactList}\n";
  t << "{\\begin{list}{}{\n";
  t << "  \\setlength{\\leftmargin}{0.5cm}\n";
  t << "  \\setlength{\\itemsep}{0pt}\n";
  t << "  \\setlength{\\parsep}{0pt}\n";
  t << "  \\setlength{\\topsep}{0pt}\n";
  t << "  \\renewcommand{\\makelabel}{\\hfill}}}\n";
  t << "{\\end{list}}\n";
  t << "\\newenvironment{DoxyCompactItemize}\n";
  t << "{\n";
  t << "  \\begin{itemize}\n";
  t << "  \\setlength{\\itemsep}{-3pt}\n";
  t << "  \\setlength{\\parsep}{0pt}\n";
  t << "  \\setlength{\\topsep}{0pt}\n";
  t << "  \\setlength{\\partopsep}{0pt}\n";
  t << "}\n";
  t << "{\\end{itemize}}\n";
  t << "\\newcommand{\\PBS}[1]{\\let\\temp=\\\\#1\\let\\\\=\\temp}\n";
  t << "\\newlength{\\tmplength}\n";
  t << "\\newenvironment{TabularC}[1]\n";
  t << "{\n";
  t << "\\setlength{\\tmplength}\n";
  t << "     {\\linewidth/(#1)-\\tabcolsep*2-\\arrayrulewidth*(#1+1)/(#1)}\n";
  t << "      \\par\\begin{xtabular*}{\\linewidth}\n";
  t << "             {*{#1}{|>{\\PBS\\raggedright\\hspace{0pt}}p{\\the\\tmplength}}|}\n";
  t << "}\n";
  t << "{\\end{xtabular*}\\par}\n";
  t << "\\newcommand{\\entrylabel}[1]{\n";
  t << "   {\\parbox[b]{\\labelwidth-4pt}{\\makebox[0pt][l]{%\n";
  t << "   \\usefont{OT1}{phv}{bc}{n}\\color{darkgray}#1}\\vspace{1.5\\baselineskip}}}}\n";
  t << "\\newenvironment{Desc}\n";
  t << "{\\begin{list}{}\n";
  t << "  {\n";
  t << "    \\settowidth{\\labelwidth}{40pt}\n";
  t << "    \\setlength{\\leftmargin}{\\labelwidth}\n";
  t << "    \\setlength{\\parsep}{0pt}\n";
  t << "    \\setlength{\\itemsep}{-4pt}\n";
  t << "    \\renewcommand{\\makelabel}{\\entrylabel}\n";
  t << "  }\n";
  t << "}\n";
  t << "{\\end{list}}\n";

  t << "\\newsavebox{\\xrefbox}\n";
  t << "\\newlength{\\xreflength}\n";
  t << "\\newcommand{\\xreflabel}[1]{%\n";
  t << "  \\sbox{\\xrefbox}{#1}%\n";
  t << "  \\setlength{\\xreflength}{\\wd\\xrefbox}%\n";
  t << "  \\ifthenelse{\\xreflength>\\labelwidth}{%\n";
  t << "    \\begin{minipage}{\\textwidth}%\n";
  t << "      \\setlength{\\parindent}{0pt}%\n";
  t << "      \\hangindent=15pt\\bfseries #1\\vspace{1.2\\itemsep}%\n";
  t << "    \\end{minipage}%\n";
  t << "  }{%\n";
  t << "   \\parbox[b]{\\labelwidth}{\\makebox[0pt][l]{\\textbf{#1}}}%\n";
  t << "  }}%\n";
  t << "\\newenvironment{DoxyRefList}{%\n";
  t << "  \\begin{list}{}{%\n";
  t << "    \\setlength{\\labelwidth}{10pt}%\n";
  t << "    \\setlength{\\leftmargin}{\\labelwidth}%\n";
  t << "    \\addtolength{\\leftmargin}{\\labelsep}%\n";
  t << "    \\renewcommand{\\makelabel}{\\xreflabel}%\n";
  t << "    }%\n";
  t << "  }%\n";
  t << "{\\end{list}}\n";
  t << "\\newenvironment{DoxyRefDesc}[1]\n";
  t << "{\\begin{list}{}{%\n";
  t << "  \\renewcommand\\makelabel[1]{\\textbf{##1}}\n";
  t << "  \\settowidth\\labelwidth{\\makelabel{#1}}\n";
  t << "  \\setlength\\leftmargin{\\labelwidth+\\labelsep}}}\n";
  t << "{\\end{list}}\n";
  t << "\\newenvironment{Indent}\n";
  t << "  {\\begin{list}{}{\\setlength{\\leftmargin}{0.5cm}}\n";
  t << "      \\item[]\\ignorespaces}\n";
  t << "  {\\unskip\\end{list}}\n";

  t << "\\setlength{\\parindent}{0cm}\n";
  t << "\\setlength{\\parskip}{0.2cm}\n";
  t << "\\addtocounter{secnumdepth}{2}\n";
  // \sloppy should not be used, see bug 563698 
  //t << "\\sloppy\n";
  t << "\\usepackage[T1]{fontenc}\n";
  t << "\\makeatletter\n";
  t << "\\renewcommand{\\paragraph}{\\@startsection{paragraph}{4}{0ex}%\n";
  t << "   {-1.0ex}%\n";
  t << "   {1.0ex}%\n";
  t << "   {\\usefont{OT1}{phv}{bc}{n}\\color{darkgray}}}\n";
  t << "\\renewcommand{\\subparagraph}{\\@startsection{subparagraph}{5}{0ex}%\n";
  t << "   {-1.0ex}%\n";
  t << "   {1.0ex}%\n";
  t << "   {\\usefont{OT1}{phv}{bc}{n}\\color{darkgray}}}\n";
  t << "\\makeatother\n";
  t << "\\allsectionsfont{\\usefont{OT1}{phv}{bc}{n}\\selectfont\\color{darkgray}}\n";
  t << "\\stepcounter{secnumdepth}\n";
  t << "\\stepcounter{tocdepth}\n";
  t << "\\definecolor{comment}{rgb}{0.5,0.0,0.0}\n";
  t << "\\definecolor{keyword}{rgb}{0.0,0.5,0.0}\n";
  t << "\\definecolor{keywordtype}{rgb}{0.38,0.25,0.125}\n";
  t << "\\definecolor{keywordflow}{rgb}{0.88,0.5,0.0}\n";
  t << "\\definecolor{preprocessor}{rgb}{0.5,0.38,0.125}\n";
  t << "\\definecolor{stringliteral}{rgb}{0.0,0.125,0.25}\n";
  t << "\\definecolor{charliteral}{rgb}{0.0,0.5,0.5}\n";
  t << "\\definecolor{vhdldigit}{rgb}{1.0,0.0,1.0}\n";
  t << "\\definecolor{vhdlkeyword}{rgb}{0.43,0.0,0.43}\n";
  t << "\\definecolor{vhdllogic}{rgb}{1.0,0.0,0.0}\n";
  t << "\\definecolor{vhdlchar}{rgb}{0.0,0.0,0.0}\n";
}

static void writeDefaultFooter(FTextStream &t)
{
  Doxygen::citeDict->writeLatexBibliography(t);
  t << "\\printindex\n";
  t << "\\end{document}\n";
}

void LatexGenerator::writeHeaderFile(QFile &f)
{
  FTextStream t(&f);
  writeDefaultHeaderPart1(t);
  t << "Your title here";
  writeDefaultHeaderPart2(t);
  t << "Generated by";
  writeDefaultHeaderPart3(t);
}

void LatexGenerator::writeFooterFile(QFile &f)
{
  FTextStream t(&f);
  writeDefaultFooter(t);
}

void LatexGenerator::writeStyleSheetFile(QFile &f)
{
  FTextStream t(&f);

  writeDefaultStyleSheetPart1(t);
  QCString &projectName = Config_getString("PROJECT_NAME");

  t << theTranslator->trGeneratedAt( dateToString(TRUE), projectName );
  t << " doxygen";
  //t << " " << theTranslator->trWrittenBy() << " ";
  //t << "Dimitri van Heesch \\copyright~1997-2012";
  writeDefaultStyleSheetPart2(t);
  t << theTranslator->trGeneratedAt( dateToString(TRUE), projectName );
  t << " doxygen";
  //t << " << theTranslator->trWrittenBy() << " ";
  //t << "Dimitri van Heesch \\copyright~1997-2012";
  writeDefaultStyleSheetPart3(t);
}

void LatexGenerator::startFile(const char *name,const char *,const char *)
{
#if 0
  setEncoding(Config_getString("LATEX_OUTPUT_ENCODING"));
#endif
  QCString fileName=name;
  relPath = relativePathToRoot(fileName);
  sourceFileName = stripPath(fileName);
  if (fileName.right(4)!=".tex" && fileName.right(4)!=".sty") fileName+=".tex";
  startPlainFile(fileName);
}

void LatexGenerator::endFile()
{
  endPlainFile();
  sourceFileName.resize(0);
}

//void LatexGenerator::writeIndex()
//{
//  startFile("refman.tex");
//} 
  
void LatexGenerator::startProjectNumber()
{
  t << "\\\\[1ex]\\large "; 
}

void LatexGenerator::startIndexSection(IndexSections is)
{
  bool &compactLatex = Config_getBool("COMPACT_LATEX");
  QCString &latexHeader = Config_getString("LATEX_HEADER");
  switch (is)
  {
    case isTitlePageStart:
      {
        if (latexHeader.isEmpty())
        {
          writeDefaultHeaderPart1(t);
        }
        else
        {
          QCString header = fileToString(latexHeader);
          t << substituteKeywords(header,0,
              Config_getString("PROJECT_NAME"),
              Config_getString("PROJECT_NUMBER"),
              Config_getString("PROJECT_BRIEF"));
        }
      }
      break;
    case isTitlePageAuthor:
      if (latexHeader.isEmpty())
      {
        writeDefaultHeaderPart2(t);
      }
      break;
    case isMainPage:
      if (compactLatex) t << "\\section"; else t << "\\chapter";
      t << "{"; //Introduction}\n"
      break;
    //case isPackageIndex:
    //  if (compactLatex) t << "\\section"; else t << "\\chapter";
    //  t << "{"; //Package Index}\n"
    //  break;
    case isModuleIndex:
      if (compactLatex) t << "\\section"; else t << "\\chapter";
      t << "{"; //Module Index}\n"
      break;
    case isDirIndex:
      if (compactLatex) t << "\\section"; else t << "\\chapter";
      t << "{"; //Directory Index}\n"
      break;
    case isNamespaceIndex:
      if (compactLatex) t << "\\section"; else t << "\\chapter";
      t << "{"; //Namespace Index}\"
      break;
    case isClassHierarchyIndex:
      if (compactLatex) t << "\\section"; else t << "\\chapter";
      t << "{"; //Hierarchical Index}\n"
      break;
    case isCompoundIndex:
      if (compactLatex) t << "\\section"; else t << "\\chapter";
      t << "{"; //Annotated Compound Index}\n"
      break;
    case isFileIndex:
      if (compactLatex) t << "\\section"; else t << "\\chapter";
      t << "{"; //Annotated File Index}\n"
      break;
    case isPageIndex:
      if (compactLatex) t << "\\section"; else t << "\\chapter";
      t << "{"; //Annotated Page Index}\n"
      break;
    case isModuleDocumentation:
      {
        GroupSDict::Iterator gli(*Doxygen::groupSDict);
        GroupDef *gd;
        bool found=FALSE;
        for (gli.toFirst();(gd=gli.current()) && !found;++gli)
        {
          if (!gd->isReference())
          {
            if (compactLatex) t << "\\section"; else t << "\\chapter";
            t << "{"; //Module Documentation}\n";
            found=TRUE;
          }
        }
      }
      break;
    case isDirDocumentation:
      {
        SDict<DirDef>::Iterator dli(*Doxygen::directories);
        DirDef *dd;
        bool found=FALSE;
        for (dli.toFirst();(dd=dli.current()) && !found;++dli)
        {
          if (dd->isLinkableInProject())
          {
            if (compactLatex) t << "\\section"; else t << "\\chapter";
            t << "{"; //Module Documentation}\n";
            found=TRUE;
          }
        }
      }
      break;
    case isNamespaceDocumentation:
      {
        NamespaceSDict::Iterator nli(*Doxygen::namespaceSDict);
        NamespaceDef *nd;
        bool found=FALSE;
        for (nli.toFirst();(nd=nli.current()) && !found;++nli)
        {
          if (nd->isLinkableInProject())
          {
            if (compactLatex) t << "\\section"; else t << "\\chapter";
            t << "{"; // Namespace Documentation}\n":
            found=TRUE;
          }
        } 
      }
      break;
    case isClassDocumentation:
      {
        ClassSDict::Iterator cli(*Doxygen::classSDict);
        ClassDef *cd=0;
        bool found=FALSE;
        for (cli.toFirst();(cd=cli.current()) && !found;++cli)
        {
          if (cd->isLinkableInProject() && 
              cd->templateMaster()==0 &&
              !cd->isEmbeddedInOuterScope()
             )
          {
            if (compactLatex) t << "\\section"; else t << "\\chapter";
            t << "{"; //Compound Documentation}\n";
            found=TRUE;
          }
        }
      }
      break;
    case isFileDocumentation:
      {
        bool isFirst=TRUE;
        FileName *fn=Doxygen::inputNameList->first();
        while (fn)
        {
          FileDef *fd=fn->first();
          while (fd)
          {
            if (fd->isLinkableInProject())
            {
              if (isFirst)
              {
                if (compactLatex) t << "\\section"; else t << "\\chapter";
                t << "{"; //File Documentation}\n";
                isFirst=FALSE;
                break;
              }
            }
            fd=fn->next();
          }
          fn=Doxygen::inputNameList->next();
        }
      }
      break;
    case isExampleDocumentation:
      {
        if (compactLatex) t << "\\section"; else t << "\\chapter";
        t << "{"; //Example Documentation}\n";
      }
      break;
    case isPageDocumentation:
      {
        if (compactLatex) t << "\\section"; else t << "\\chapter";
        t << "{"; //Page Documentation}\n";
      }
      break;
    case isPageDocumentation2:
      break;
    case isEndIndex:
      break;
  }
}

void LatexGenerator::endIndexSection(IndexSections is)
{
  //static bool compactLatex = Config_getBool("COMPACT_LATEX");
  static bool sourceBrowser = Config_getBool("SOURCE_BROWSER");
  static QCString latexHeader = Config_getString("LATEX_HEADER");
  static QCString latexFooter = Config_getString("LATEX_FOOTER");
  switch (is)
  {
    case isTitlePageStart:
      break;
    case isTitlePageAuthor:
      if (latexHeader.isEmpty())
      {
        writeDefaultHeaderPart3(t);
      }
      break;
    case isMainPage:
      {
        //QCString indexName=Config_getBool("GENERATE_TREEVIEW")?"main":"index";
        QCString indexName="index";
        t << "}\n\\label{index}";
        if (Config_getBool("PDF_HYPERLINKS")) t << "\\hypertarget{index}{}";
        t << "\\input{" << indexName << "}\n";
      }
      break;
    case isModuleIndex:
      t << "}\n\\input{modules}\n";
      break;
    case isDirIndex:
      t << "}\n\\input{dirs}\n";
      break;
    case isNamespaceIndex:
      t << "}\n\\input{namespaces}\n";
      break;
    case isClassHierarchyIndex:
      t << "}\n\\input{hierarchy}\n";
      break;
    case isCompoundIndex:
      t << "}\n\\input{annotated}\n";
      break;
    case isFileIndex:
      t << "}\n\\input{files}\n";
      break;
    case isPageIndex:
      t << "}\n\\input{pages}\n";
      break;
    case isModuleDocumentation:
      {
        GroupSDict::Iterator gli(*Doxygen::groupSDict);
        GroupDef *gd;
        bool found=FALSE;
        for (gli.toFirst();(gd=gli.current()) && !found;++gli)
        {
          if (!gd->isReference())
          {
            t << "}\n\\input{" << gd->getOutputFileBase() << "}\n";
            found=TRUE;
          }
        }
        for (;(gd=gli.current());++gli)
        {
          if (!gd->isReference())
          {
            //if (compactLatex) t << "\\input"; else t << "\\include";
            t << "\\include"; 
            t << "{" << gd->getOutputFileBase() << "}\n";
          }
        }
      }
      break;
    case isDirDocumentation:
      {
        SDict<DirDef>::Iterator dli(*Doxygen::directories);
        DirDef *dd;
        bool found=FALSE;
        for (dli.toFirst();(dd=dli.current()) && !found;++dli)
        {
          if (dd->isLinkableInProject())
          {
            t << "}\n\\input{" << dd->getOutputFileBase() << "}\n";
            found=TRUE;
          }
        }
        for (;(dd=dli.current());++dli)
        {
          if (dd->isLinkableInProject())
          {
            //if (compactLatex) t << "\\input"; else t << "\\include";
            t << "\\input"; 
            t << "{" << dd->getOutputFileBase() << "}\n";
          }
        }
      }
      break;
    case isNamespaceDocumentation:
      {
        NamespaceSDict::Iterator nli(*Doxygen::namespaceSDict);
        NamespaceDef *nd;
        bool found=FALSE;
        for (nli.toFirst();(nd=nli.current()) && !found;++nli)
        {
          if (nd->isLinkableInProject())
          {
            t << "}\n\\input{" << nd->getOutputFileBase() << "}\n";
            found=TRUE;
          }
        }
        while ((nd=nli.current()))
        {
          if (nd->isLinkableInProject())
          {
            //if (compactLatex) t << "\\input"; else t << "\\include";
            t << "\\input"; 
            t << "{" << nd->getOutputFileBase() << "}\n";
          }
          ++nli;
        }
      }
      break;
    case isClassDocumentation:
      {
        ClassSDict::Iterator cli(*Doxygen::classSDict);
        ClassDef *cd=0;
        bool found=FALSE;
        for (cli.toFirst();(cd=cli.current()) && !found;++cli)
        {
          if (cd->isLinkableInProject() && 
              cd->templateMaster()==0 &&
             !cd->isEmbeddedInOuterScope()
             )
          {
            t << "}\n\\input{" << cd->getOutputFileBase() << "}\n";
            found=TRUE;
          }
        }
        for (;(cd=cli.current());++cli)
        {
          if (cd->isLinkableInProject() && 
              cd->templateMaster()==0 &&
             !cd->isEmbeddedInOuterScope()
             )
          {
            //if (compactLatex) t << "\\input"; else t << "\\include";
            t << "\\input"; 
            t << "{" << cd->getOutputFileBase() << "}\n";
          } 
        }
      }
      break;
    case isFileDocumentation:
      {
        bool isFirst=TRUE;
        FileName *fn=Doxygen::inputNameList->first();
        while (fn)
        {
          FileDef *fd=fn->first();
          while (fd)
          {
            if (fd->isLinkableInProject())
            {
              if (isFirst)
              {
                t << "}\n\\input{" << fd->getOutputFileBase() << "}\n";
                if (sourceBrowser && m_prettyCode && fd->generateSourceFile())
                {
                  //t << "\\include{" << fd->getSourceFileBase() << "}\n";
                  t << "\\input{" << fd->getSourceFileBase() << "}\n";
                }
                isFirst=FALSE;
              }
              else
              {
                //if (compactLatex) t << "\\input" ; else t << "\\include";
                t << "\\input" ; 
                t << "{" << fd->getOutputFileBase() << "}\n";
                if (sourceBrowser && m_prettyCode && fd->generateSourceFile())
                {
                  //t << "\\include{" << fd->getSourceFileBase() << "}\n";
                  t << "\\input{" << fd->getSourceFileBase() << "}\n";
                }
              }
            }
            fd=fn->next();
          }
          fn=Doxygen::inputNameList->next();
        }
      }
      break;
    case isExampleDocumentation:
      {
        t << "}\n";
        PageSDict::Iterator pdi(*Doxygen::exampleSDict);
        PageDef *pd=pdi.toFirst();
        if (pd)
        {
          t << "\\input{" << pd->getOutputFileBase() << "}\n";
        }
        for (++pdi;(pd=pdi.current());++pdi)
        {
          //if (compactLatex) t << "\\input" ; else t << "\\include";
          t << "\\input"; 
          t << "{" << pd->getOutputFileBase() << "}\n";
        }
      }
      break;
    case isPageDocumentation:
      {
        t << "}\n";
#if 0
        PageSDict::Iterator pdi(*Doxygen::pageSDict);
        PageDef *pd=pdi.toFirst();
        bool first=TRUE;
        for (pdi.toFirst();(pd=pdi.current());++pdi)
        {
          if (!pd->getGroupDef() && !pd->isReference())
          {
             if (compactLatex) t << "\\section"; else t << "\\chapter";
             t << "{" << pd->title();
             t << "}\n";
            
            if (compactLatex || first) t << "\\input" ; else t << "\\include";
            t << "{" << pd->getOutputFileBase() << "}\n";
            first=FALSE;
          }
        }
#endif
      }
      break;
    case isPageDocumentation2:
      break;
    case isEndIndex:
      if (latexFooter.isEmpty())
      {
        writeDefaultFooter(t);
      }
      else
      {
        QCString footer = fileToString(latexFooter);
        t << substituteKeywords(footer,0,
              Config_getString("PROJECT_NAME"),
              Config_getString("PROJECT_NUMBER"),
              Config_getString("PROJECT_BRIEF"));
      }
      break;
  }
}

void LatexGenerator::writePageLink(const char *name, bool /*first*/)
{
  //bool &compactLatex = Config_getBool("COMPACT_LATEX");
  // next is remove for bug615957
  //if (compactLatex || first) t << "\\input" ; else t << "\\include";
  t << "\\input" ; 
  t << "{" << name << "}\n";
}


void LatexGenerator::writeStyleInfo(int part)
{
  switch(part)
  {
    case 0:
      {
        //QCString pname=Config_getString("PROJECT_NAME").stripWhiteSpace();
        startPlainFile("doxygen.sty");
        writeDefaultStyleSheetPart1(t);
      }
      break;
    case 1:
    case 3:
      t << " Doxygen ";
      break;
    case 2:
      {
        writeDefaultStyleSheetPart2(t);
      }
      break;
    case 4:
      {
        writeDefaultStyleSheetPart3(t);
        endPlainFile();
      }
      break;
  }
}

void LatexGenerator::newParagraph()
{
  t << endl << endl;
}

void LatexGenerator::startParagraph()
{
  t << endl << endl;
}

void LatexGenerator::endParagraph()
{
  t << endl << endl;
}

void LatexGenerator::writeString(const char *text)
{
  t << text;
}

void LatexGenerator::startIndexItem(const char *ref,const char *fn)
{
  t << "\\item ";
  if (!ref && fn)
  {
    t << "\\contentsline{section}{";
  }
}

void LatexGenerator::endIndexItem(const char *ref,const char *fn)
{
  if (!ref && fn)
  {
    t << "}{\\pageref{" << fn << "}}{}" << endl;
  }
}

//void LatexGenerator::writeIndexFileItem(const char *,const char *text)
//{
//  t << "\\item\\contentsline{section}{";
//  docify(text);
//  t << "}{\\pageref{" << text << "}}" << endl;
//}


void LatexGenerator::startHtmlLink(const char *url)
{
  if (Config_getBool("PDF_HYPERLINKS"))
  {
    t << "\\href{";
    t << url;
    t << "}";
  }
  t << "{\\tt ";
}

void LatexGenerator::endHtmlLink()
{
  t << "}";
}

//void LatexGenerator::writeMailLink(const char *url)
//{
//  if (Config_getBool("PDF_HYPERLINKS"))
//  {
//    t << "\\href{mailto:";
//    t << url;
//    t << "}";
//  }
//  t << "{\\tt "; 
//  docify(url);
//  t << "}";
//}

void LatexGenerator::writeStartAnnoItem(const char *,const char *,
                                        const char *path,const char *name)
{
  t << "\\item\\contentsline{section}{\\bf ";
  if (path) docify(path);
  docify(name); 
  t << "} ";
}

void LatexGenerator::writeEndAnnoItem(const char *name)
{
  t << "}{\\pageref{" << name << "}}{}" << endl;
}

void LatexGenerator::startIndexKey()
{
  t << "\\item\\contentsline{section}{";
}

void LatexGenerator::endIndexKey()
{
}

void LatexGenerator::startIndexValue(bool hasBrief)
{
  t << " ";
  if (hasBrief) t << "\\\\*";
}

void LatexGenerator::endIndexValue(const char *name,bool /*hasBrief*/)
{
  //if (hasBrief) t << ")";
  t << "}{\\pageref{" << name << "}}{}" << endl;
}

//void LatexGenerator::writeClassLink(const char *,const char *,
//                                    const char *,const char *name)
//{
//  t << "{\\bf ";
//  docify(name);
//  t << "}"; 
//}

void LatexGenerator::startTextLink(const char *f,const char *anchor)
{
  if (!disableLinks && Config_getBool("PDF_HYPERLINKS"))
  {
    t << "\\hyperlink{";
    if (f) t << stripPath(f);
    if (anchor) t << "_" << anchor; 
    t << "}{";
  }
  else
  {
    t << "{\\bf ";
  }
}

void LatexGenerator::endTextLink()
{
  t << "}";
}

void LatexGenerator::writeObjectLink(const char *ref, const char *f,
                                     const char *anchor, const char *text)
{
  if (!disableLinks && !ref && Config_getBool("PDF_HYPERLINKS"))
  {
    t << "\\hyperlink{";
    if (f) t << stripPath(f);
    if (f && anchor) t << "_"; 
    if (anchor) t << anchor; 
    t << "}{";
    docify(text);
    t << "}";
  }
  else
  {
    t << "{\\bf ";
    docify(text);
    t << "}";
  } 
}

void LatexGenerator::startPageRef()
{
  t << " \\doxyref{}{";
}

void LatexGenerator::endPageRef(const char *clname, const char *anchor)
{
  t << "}{";
  if (clname) t << clname; 
  if (anchor) t << "_" << anchor;
  t << "}";
}

void LatexGenerator::writeCodeLink(const char *ref,const char *f,
                                   const char *anchor,const char *name,
                                   const char *)
{
  static bool pdfHyperlinks = Config_getBool("PDF_HYPERLINKS");
  static bool usePDFLatex   = Config_getBool("USE_PDFLATEX");
  int l = strlen(name);
  if (col+l>80)
  {
    t << "\n      ";
    col=0;
  }
  if (m_prettyCode && !disableLinks && !ref && usePDFLatex && pdfHyperlinks)
  {
    t << "\\hyperlink{";
    if (f) t << stripPath(f);
    if (f && anchor) t << "_"; 
    if (anchor) t << anchor; 
    t << "}{" << name << "}";
  }
  else
  {
    t << name;
  }
  col+=l;
}

void LatexGenerator::startTitleHead(const char *fileName)
{
  static bool pdfHyperlinks = Config_getBool("PDF_HYPERLINKS");
  static bool usePDFLatex   = Config_getBool("USE_PDFLATEX");
  if (usePDFLatex && pdfHyperlinks && fileName)
  {
    t << "\\hypertarget{" << stripPath(fileName) << "}{";
  }
  if (Config_getBool("COMPACT_LATEX")) 
  {
    t << "\\subsection{"; 
  }
  else 
  {
    t << "\\section{"; 
  }
}

void LatexGenerator::endTitleHead(const char *fileName,const char *name)
{
  static bool pdfHyperlinks = Config_getBool("PDF_HYPERLINKS");
  static bool usePDFLatex   = Config_getBool("USE_PDFLATEX");
  t << "}" << endl;
  if (name)
  {
    t << "\\label{" << fileName << "}\\index{";
    escapeLabelName(name);
    t << "@{";
    escapeMakeIndexChars(name);
    t << "}}" << endl;
  }
  if (usePDFLatex && pdfHyperlinks && fileName)
  {
    t << "}" << endl;
  }
}

void LatexGenerator::startTitle()
{
  if (Config_getBool("COMPACT_LATEX")) 
  {
    t << "\\subsection{"; 
  }
  else 
  {
    t << "\\section{"; 
  }
}

void LatexGenerator::startGroupHeader(int extraIndentLevel)
{
  if (Config_getBool("COMPACT_LATEX")) 
  {
    extraIndentLevel++;
  }

  if (extraIndentLevel==3)
  {
    t << "\\subparagraph*{"; 
  }
  else if (extraIndentLevel==2)
  {
    t << "\\paragraph{";
  }
  else if (extraIndentLevel==1)
  {
    t << "\\subsubsection{";
  }
  else // extraIndentLevel==0
  {
    t << "\\subsection{";
  }
  disableLinks=TRUE;
}

void LatexGenerator::endGroupHeader(int)
{
  disableLinks=FALSE;
  t << "}" << endl;
}

void LatexGenerator::startMemberHeader(const char *)
{
  if (Config_getBool("COMPACT_LATEX")) 
  {
    t << "\\subsubsection*{"; 
  }
  else 
  {
    t << "\\subsection*{";
  }
  disableLinks=TRUE;
}

void LatexGenerator::endMemberHeader()
{
  disableLinks=FALSE;
  t << "}" << endl;
}

void LatexGenerator::startMemberDoc(const char *clname,
                                    const char *memname,
                                    const char *,
                                    const char *title,
                                    bool showInline)
{ 
  if (memname && memname[0]!='@')
  {
    t << "\\index{";
    if (clname)
    {
      escapeLabelName(clname);
      t << "@{";
      escapeMakeIndexChars(clname);
      t << "}!";
    }
    escapeLabelName(memname);
    t << "@{";
    escapeMakeIndexChars(memname);
    t << "}}" << endl;

    t << "\\index{";
    escapeLabelName(memname);
    t << "@{";
    escapeMakeIndexChars(memname);
    t << "}";
    if (clname)
    {
      t << "!" << clname << "@{";
      docify(clname);
      t << "}"; 
    }
    t << "}" << endl;
  }
  static const char *levelLab[] = { "subsubsection","paragraph","subparagraph", "subparagraph" };
  static bool compactLatex = Config_getBool("COMPACT_LATEX");
  int level=0;
  if (showInline) level+=2;
  if (compactLatex) level++;
  t << "\\" << levelLab[level]; 

  //if (Config_getBool("PDF_HYPERLINKS") && memname) 
  //{
  //  t << "["; 
  //  escapeMakeIndexChars(this,t,memname);
  //  t << "]";
  //}
  t << "[{";
  escapeMakeIndexChars(title);
  t << "}]";
  t << "{\\setlength{\\rightskip}{0pt plus 5cm}";
  disableLinks=TRUE;
}

void LatexGenerator::endMemberDoc(bool) 
{ 
  disableLinks=FALSE;
  t << "}";
  //if (Config_getBool("COMPACT_LATEX")) t << "\\hfill";
}

void LatexGenerator::startDoxyAnchor(const char *fName,const char *,
                                     const char *anchor, const char *,
                                     const char *)
{
  static bool pdfHyperlinks = Config_getBool("PDF_HYPERLINKS");
  static bool usePDFLatex   = Config_getBool("USE_PDFLATEX");
  if (usePDFLatex && pdfHyperlinks)
  {
    t << "\\hypertarget{";
    if (fName) t << stripPath(fName);
    if (anchor) t << "_" << anchor;
    t << "}{";
  }
}

void LatexGenerator::endDoxyAnchor(const char *fName,const char *anchor)
{
  static bool pdfHyperlinks = Config_getBool("PDF_HYPERLINKS");
  static bool usePDFLatex   = Config_getBool("USE_PDFLATEX");
  if (usePDFLatex && pdfHyperlinks)
  {
    t << "}";
  }
  t << "\\label{";
  if (fName) t << fName;
  if (anchor) t << "_" << anchor;
  t << "}" << endl;
}

void LatexGenerator::writeAnchor(const char *fName,const char *name)
{ 
  //printf("LatexGenerator::writeAnchor(%s,%s)\n",fName,name);
  t << "\\label{" << name << "}" << endl; 
  static bool pdfHyperlinks = Config_getBool("PDF_HYPERLINKS");
  static bool usePDFLatex   = Config_getBool("USE_PDFLATEX");
  if (usePDFLatex && pdfHyperlinks)
  {
    if (fName)
    {
      t << "\\hypertarget{" << stripPath(fName) << "_" << name << "}{}" << endl;
    }
    else
    {
      t << "\\hypertarget{" << name << "}{}" << endl;
    }
  }
}


//void LatexGenerator::writeLatexLabel(const char *clName,const char *anchor)
//{
//  writeDoxyAnchor(0,clName,anchor,0);
//}

void LatexGenerator::addIndexItem(const char *s1,const char *s2)
{
  if (s1)
  {
    t << "\\index{";
    escapeLabelName(s1);
    t << "@{";
    escapeMakeIndexChars(s1);
    t << "}";
    if (s2)
    {
      t << "!";
      escapeLabelName(s2);
      t << "@{";
      escapeMakeIndexChars(s2);
      t << "}";
    }
    t << "}";
  }
}


void LatexGenerator::startSection(const char *lab,const char *,SectionInfo::SectionType type)
{
  static bool pdfHyperlinks = Config_getBool("PDF_HYPERLINKS");
  static bool usePDFLatex   = Config_getBool("USE_PDFLATEX");
  if (usePDFLatex && pdfHyperlinks)
  {
    t << "\\hypertarget{" << stripPath(lab) << "}{}";
  }
  t << "\\";
  if (Config_getBool("COMPACT_LATEX"))
  {
    switch(type)
    {
      case SectionInfo::Page:          t << "subsection"; break;
      case SectionInfo::Section:       t << "subsubsection"; break;
      case SectionInfo::Subsection:    t << "paragraph"; break;
      case SectionInfo::Subsubsection: t << "subparagraph"; break;
      case SectionInfo::Paragraph:     t << "subparagraph"; break;
      default: ASSERT(0); break;
    }
    t << "{";
  }
  else
  {
    switch(type)
    {
      case SectionInfo::Page:          t << "section"; break;
      case SectionInfo::Section:       t << "subsection"; break;
      case SectionInfo::Subsection:    t << "subsubsection"; break;
      case SectionInfo::Subsubsection: t << "paragraph"; break;
      case SectionInfo::Paragraph:     t << "subparagraph"; break;
      default: ASSERT(0); break;
    }
    t << "{";
  }
}

void LatexGenerator::endSection(const char *lab,SectionInfo::SectionType)
{
  t << "}\\label{" << lab << "}" << endl;
}


void LatexGenerator::docify(const char *str)
{
  filterLatexString(t,str,insideTabbing,FALSE,FALSE);
}

void LatexGenerator::codify(const char *str)
{
  if (str)
  { 
    const char *p=str;
    char c;
    //char cs[5];
    int spacesToNextTabStop;
    static int tabSize = Config_getInt("TAB_SIZE");
    const int maxLineLen = 80;
    QCString result(4*maxLineLen+1); // worst case for 1 line of 4-byte chars
    int i;
    while ((c=*p))
    {
      switch(c)
      {
        case 0x0c: p++;  // remove ^L
                   break;
        case '\t': spacesToNextTabStop =
                         tabSize - (col%tabSize);
                   t << Doxygen::spaces.left(spacesToNextTabStop); 
                   col+=spacesToNextTabStop;
                   p++;
                   break; 
        case '\n': t << '\n'; col=0; p++;
                   break;
        default:   
                   i=0;

#undef  COPYCHAR
// helper macro to copy a single utf8 character, dealing with multibyte chars.
#define COPYCHAR() do {                                           \
                     result[i++]=c; p++;                          \
                     if (c<0) /* multibyte utf-8 character */     \
                     {                                            \
                       /* 1xxx.xxxx: >=2 byte character */        \
                       result[i++]=*p++;                          \
                       if (((uchar)c&0xE0)==0xE0)                 \
                       {                                          \
                         /* 111x.xxxx: >=3 byte character */      \
                         result[i++]=*p++;                        \
                       }                                          \
                       if (((uchar)c&0xF0)==0xF0)                 \
                       {                                          \
                         /* 1111.xxxx: 4 byte character */        \
                         result[i++]=*p++;                        \
                       }                                          \
                     }                                            \
                     col++;                                       \
                   } while(0)

                   // gather characters until we find whitespace or are at
                   // the end of a line
                   COPYCHAR();
                   if (col>=maxLineLen) // force line break
                   {
                     t << "\n      ";
                     col=0;
                   }
                   else // copy more characters
                   {
                     while (col<maxLineLen && (c=*p) && 
                            c!=0x0c && c!='\t' && c!='\n' && c!=' '
                           )
                     {
                       COPYCHAR();
                     }
                     if (col>=maxLineLen) // force line break
                     {
                       t << "\n      ";
                       col=0;
                     }
                   }
                   result[i]=0; // add terminator
                   if (m_prettyCode)
                   {
                     filterLatexString(t,result,insideTabbing,TRUE);
                   }
                   else
                   {
                     t << result;
                   }
                   break;
      }
    }
  }
}

void LatexGenerator::writeChar(char c)
{
  char cs[2];
  cs[0]=c;
  cs[1]=0;
  docify(cs);
}

void LatexGenerator::startClassDiagram()
{
  //if (Config_getBool("COMPACT_LATEX")) t << "\\subsubsection"; else t << "\\subsection";
  //t << "{";
}

void LatexGenerator::endClassDiagram(const ClassDiagram &d,
                                       const char *fileName,const char *)
{
  d.writeFigure(t,dir,fileName);
}


void LatexGenerator::startAnonTypeScope(int indent)
{
  if (indent==0)
  {
    t << "\\begin{tabbing}" << endl;
    t << "xx\\=xx\\=xx\\=xx\\=xx\\=xx\\=xx\\=xx\\=xx\\=\\kill" << endl;
    insideTabbing=TRUE;
  }
  m_indent=indent;
}

void LatexGenerator::endAnonTypeScope(int indent)
{
  if (indent==0)
  {
    t << endl << "\\end{tabbing}";
    insideTabbing=FALSE;
  }
  m_indent=indent;
}

void LatexGenerator::startMemberTemplateParams()
{
  if (templateMemberItem)
  {
    t << "{\\footnotesize ";
  }
}

void LatexGenerator::endMemberTemplateParams(const char *)
{
  if (templateMemberItem)
  {
    t << "}\\\\";
  }
}

void LatexGenerator::startMemberItem(const char *,int annoType) 
{ 
  //printf("LatexGenerator::startMemberItem(%d)\n",annType);
  if (!insideTabbing)
  {
    t << "\\item " << endl; 
    templateMemberItem = (annoType == 3);
  }
}

void LatexGenerator::endMemberItem() 
{
  if (insideTabbing)
  {
    t << "\\\\";
  } 
  templateMemberItem = FALSE;
  t << endl; 
}

void LatexGenerator::startMemberDescription(const char *) 
{
  if (!insideTabbing)
  { 
    t << "\\begin{DoxyCompactList}\\small\\item\\em "; 
  }
  else
  {
    for (int i=0;i<m_indent+2;i++) t << "\\>";
    t << "{\\em ";
  }
}

void LatexGenerator::endMemberDescription() 
{ 
  if (!insideTabbing)
  {
    //t << "\\item\\end{DoxyCompactList}"; 
    t << "\\end{DoxyCompactList}"; 
  }
  else
  {
    t << "}\\\\\n";
  }
}


void LatexGenerator::writeNonBreakableSpace(int) 
{
  //printf("writeNonBreakbleSpace()\n");
  if (insideTabbing)
  {
    t << "\\>";
  }
  else
  {
    t << "~"; 
  }
}

void LatexGenerator::startMemberList()  
{ 
  if (!insideTabbing)
  {
    t << "\\begin{DoxyCompactItemize}" << endl; 
  }
}

void LatexGenerator::endMemberList()    
{
  //printf("LatexGenerator::endMemberList(%d)\n",insideTabbing);
  if (!insideTabbing)
  {
    t << "\\end{DoxyCompactItemize}"   << endl; 
  }
}


void LatexGenerator::startMemberGroupHeader(bool hasHeader)
{
  if (hasHeader) t << "\\begin{Indent}";
  t << "{\\bf ";
  // changed back to rev 756 due to bug 660501
  //if (Config_getBool("COMPACT_LATEX")) 
  //{
  //  t << "\\subparagraph*{";
  //}
  //else
  //{
  //  t << "\\paragraph*{";
  //}
}

void LatexGenerator::endMemberGroupHeader()
{
  // changed back to rev 756 due to bug 660501
  t << "}\\par" << endl;
  //t << "}" << endl;
}

void LatexGenerator::startMemberGroupDocs()
{
  t << "{\\em ";
}

void LatexGenerator::endMemberGroupDocs()
{
  t << "}";
}

void LatexGenerator::startMemberGroup()
{
}

void LatexGenerator::endMemberGroup(bool hasHeader)
{
  if (hasHeader)t << "\\end{Indent}"; 
  t << endl;
}

void LatexGenerator::startDotGraph() 
{
  newParagraph();
}

void LatexGenerator::endDotGraph(const DotClassGraph &g) 
{
  g.writeGraph(t,EPS,Config_getString("LATEX_OUTPUT"),fileName,relPath);
}

void LatexGenerator::startInclDepGraph() 
{
}

void LatexGenerator::endInclDepGraph(const DotInclDepGraph &g) 
{
  g.writeGraph(t,EPS,Config_getString("LATEX_OUTPUT"),fileName,relPath);
}

void LatexGenerator::startGroupCollaboration() 
{
}

void LatexGenerator::endGroupCollaboration(const DotGroupCollaboration &g) 
{
  g.writeGraph(t,EPS,Config_getString("LATEX_OUTPUT"),fileName,relPath);
}

void LatexGenerator::startCallGraph() 
{
}

void LatexGenerator::endCallGraph(const DotCallGraph &g) 
{
  g.writeGraph(t,EPS,Config_getString("LATEX_OUTPUT"),fileName,relPath);
}

void LatexGenerator::startDirDepGraph() 
{
}

void LatexGenerator::endDirDepGraph(const DotDirDeps &g) 
{
  g.writeGraph(t,EPS,Config_getString("LATEX_OUTPUT"),fileName,relPath);
}

void LatexGenerator::startDescription() 
{ 
  t << "\\begin{description}" << endl; 
}

void LatexGenerator::endDescription()   
{ 
  t << "\\end{description}" << endl; 
  firstDescItem=TRUE;
}

void LatexGenerator::startDescItem()    
{ 
  firstDescItem=TRUE;
  t << "\\item["; 
}

void LatexGenerator::endDescItem()      
{ 
  if (firstDescItem) 
  {
    t << "]" << endl;
    firstDescItem=FALSE;
  } 
  else
  {
    lineBreak();
  }
}

void LatexGenerator::startSimpleSect(SectionTypes,const char *file,
                                     const char *anchor,const char *title)
{
  t << "\\begin{Desc}\n\\item[";
  if (file)
  {
    writeObjectLink(0,file,anchor,title);
  }
  else
  {
    docify(title);
  }
  t << "]";
}

void LatexGenerator::endSimpleSect()
{
  t << "\\end{Desc}" << endl;
}

void LatexGenerator::startParamList(ParamListTypes,const char *title)
{
  t << "\\begin{Desc}\n\\item[";
  docify(title);
  t << "]";
}

void LatexGenerator::endParamList()
{
  t << "\\end{Desc}" << endl;
}

void LatexGenerator::startParameterList(bool openBracket)
{
  /* start of ParameterType ParameterName list */
  if (openBracket) t << "(";
  t << endl << "\\begin{DoxyParamCaption}" << endl;
}

void LatexGenerator::endParameterList()
{
}

void LatexGenerator::startParameterType(bool first,const char *key)
{
  t << "\\item[{";
  if (!first && key) t << key;
}

void LatexGenerator::endParameterType()
{
  t << "}]";
}

void LatexGenerator::startParameterName(bool /*oneArgOnly*/)
{
  t << "{";
}

void LatexGenerator::endParameterName(bool last,bool /* emptyList */,bool closeBracket)
{
  t << "}" << endl;

  if (last)
  {
    t << "\\end{DoxyParamCaption}" << endl;
    if (closeBracket) t << ")";
  }
}


void LatexGenerator::printDoc(DocNode *n,const char *langExt)
{
  LatexDocVisitor *visitor = new LatexDocVisitor(t,*this,langExt,insideTabbing);
  n->accept(visitor);
  delete visitor; 
}

void LatexGenerator::startConstraintList(const char *header)
{
  t << "\\begin{Desc}\n\\item[";
  docify(header);
  t << "]";
  t << "\\begin{description}" << endl;
}

void LatexGenerator::startConstraintParam()
{
  t << "\\item[{\\em ";
}

void LatexGenerator::endConstraintParam()
{
}

void LatexGenerator::startConstraintType()
{
  t << "} : {\\em ";
}

void LatexGenerator::endConstraintType()
{
  t << "}]";
}

void LatexGenerator::startConstraintDocs()
{
}

void LatexGenerator::endConstraintDocs()
{
}

void LatexGenerator::endConstraintList()
{
  t << "\\end{description}" << endl;
  t << "\\end{Desc}" << endl;
}

void LatexGenerator::escapeLabelName(const char *s)
{
  if (s==0) return;
  const char *p=s;
  char c;
  QCString result(strlen(s)+1); // worst case allocation
  int i;
  while ((c=*p++))
  {
    switch (c)
    {
      case '%': t << "\\%";       break;
      // NOTE: adding a case here, means adding it to while below as well!
      default:  
        i=0;
        // collect as long string as possible, before handing it to docify
        result[i++]=c;
        while ((c=*p) && c!='%')
        {
          result[i++]=c;
          p++;
        }
        result[i]=0;
        docify(result); 
        break;
    }
  }
}

void LatexGenerator::escapeMakeIndexChars(const char *s)
{
  if (s==0) return;
  const char *p=s;
  char c;
  QCString result(strlen(s)+1); // worst case allocation
  int i;
  while ((c=*p++))
  {
    switch (c)
    {
      case '"': t << "\"\""; break;
      case '@': t << "\"@"; break;
      case '[': t << "["; break;
      case ']': t << "]"; break;
      // NOTE: adding a case here, means adding it to while below as well!
      default:  
        i=0;
        // collect as long string as possible, before handing it to docify
        result[i++]=c;
        while ((c=*p) && c!='"' && c!='@' && c!='[' && c!=']')
        {
          result[i++]=c;
          p++;
        }
        result[i]=0;
        docify(result); 
        break;
    }
  }
}

void LatexGenerator::startCodeFragment()
{
  t << "\n\\begin{DoxyCode}\n";
}

void LatexGenerator::endCodeFragment()
{
  t << "\\end{DoxyCode}\n";
}

void LatexGenerator::writeLineNumber(const char *ref,const char *fileName,const char *anchor,int l)
{
  if (m_prettyCode)
  {
    QCString lineNumber;
    lineNumber.sprintf("%05d",l);

    if (fileName && !sourceFileName.isEmpty())
    {
      QCString lineAnchor;
      lineAnchor.sprintf("_l%05d",l);
      lineAnchor.prepend(sourceFileName);
      startCodeAnchor(lineAnchor);
      writeCodeLink(ref,fileName,anchor,lineNumber,0);
      endCodeAnchor();
    }
    else
    { 
      codify(lineNumber);
    }
    t << " ";
  }
  else
  {
    t << l << " ";
  }
}

void LatexGenerator::startCodeLine()
{
  col=0;
}

void LatexGenerator::endCodeLine()
{
  codify("\n");
}

void LatexGenerator::startFontClass(const char *name)
{
  if (!m_prettyCode) return;
  t << "\\textcolor{" << name << "}{";
}

void LatexGenerator::endFontClass()
{
  if (!m_prettyCode) return;
  t << "}";
}

void LatexGenerator::startCodeAnchor(const char *name) 
{
  static bool usePDFLatex = Config_getBool("USE_PDFLATEX");
  static bool pdfHyperlinks = Config_getBool("PDF_HYPERLINKS");
  if (!m_prettyCode) return;
  if (usePDFLatex && pdfHyperlinks)
  {
    t << "\\hypertarget{" << stripPath(name) << "}{}";
  }
}

void LatexGenerator::endCodeAnchor() 
{
}

void LatexGenerator::startInlineHeader()
{
  if (Config_getBool("COMPACT_LATEX")) 
  {
    t << "\\paragraph*{"; 
  }
  else 
  {
    t << "\\subsubsection*{";
  }
}

void LatexGenerator::endInlineHeader()
{
  t << "}" << endl;
}

void LatexGenerator::lineBreak(const char *)
{
  if (insideTabbing)
  {
    t << "\\\\\n";
  }
  else
  {
    t << "\\\\*\n";
  }
}

void LatexGenerator::startMemberDocSimple()
{
  t << "\\begin{DoxyFields}{";
  docify(theTranslator->trCompoundMembers());
  t << "}" << endl;
}

void LatexGenerator::endMemberDocSimple()
{
  t << "\\end{DoxyFields}" << endl;
}

void LatexGenerator::startInlineMemberType()
{
}

void LatexGenerator::endInlineMemberType()
{
  t << "&" << endl;
}

void LatexGenerator::startInlineMemberName()
{
}

void LatexGenerator::endInlineMemberName()
{
  t << "&" << endl;
}

void LatexGenerator::startInlineMemberDoc()
{
}

void LatexGenerator::endInlineMemberDoc()
{
  t << "\\\\\n\\hline\n" << endl;
}

