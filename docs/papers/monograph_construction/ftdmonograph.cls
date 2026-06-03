% =============================================================================
% ftdmonograph.cls  --  A monochrome, print-safe mathematical-monograph class
% -----------------------------------------------------------------------------
% Built on memoir.  Design language:  NO COLOUR conveys meaning.  Epistemic
% tier, structural emphasis, and authority are carried entirely by typography
% and rule-work (small caps, rule weight, solid/dashed/double rules) so the
% document reads perfectly in grayscale and on a monochrome printer.  In the
% tradition of Bringhurst / Knuth / the AMS book series.
%
% Class options:
%   nomargintags   -- disable the outer-margin epistemic tags (inline only).
%
% Companion packages (loaded by the master file, not here):
%   ftd-math-macros.sty   -- canonical equations & constants (anti-drift)
%   ftd-epistemic.sty     -- the monochrome epistemic-tag system
% =============================================================================
\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{ftdmonograph}[2026/06/02 v1.0 FTD monochrome mathematical monograph]

\newif\ifftd@margintags \ftd@margintagstrue
\DeclareOption{nomargintags}{\ftd@margintagsfalse}
\DeclareOption*{\PassOptionsToClass{\CurrentOption}{memoir}}
\ProcessOptions\relax

\LoadClass[11pt,oneside,onecolumn,final]{memoir}

% -----------------------------------------------------------------------------
% Fonts and math.  Order is load-bearing:  amsmath/amsthm/mathtools BEFORE the
% newtx math font, text font BEFORE math font, microtype LAST.
% -----------------------------------------------------------------------------
\RequirePackage[T1]{fontenc}
\RequirePackage[utf8]{inputenc}
\RequirePackage{textcomp}
\RequirePackage{amsmath}
\RequirePackage{amsthm}
\RequirePackage{mathtools}
\RequirePackage{newtxtext}
\RequirePackage[varg,bigdelims]{newtxmath}
\RequirePackage[scaled=0.92]{newtxtt}
\RequirePackage[final]{microtype}

% structural helpers
\RequirePackage{booktabs}
\RequirePackage{array}
\RequirePackage{tabularx}
\RequirePackage{xltabular}       % page-breakable tables with X columns
% Stub the PDF-tagging sockets xltabular expects but this LaTeX build omits.
\providecommand{\SuspendTagging}[1]{}
\providecommand{\ResumeTagging}[1]{}
\RequirePackage{ragged2e}        % \RaggedRight for table cells & margin notes
\RequirePackage{enumitem}
\setlist{itemsep=2pt, parsep=1pt, topsep=4pt}

% -----------------------------------------------------------------------------
% Page geometry.  Octavo-ish trim with a wide OUTER margin to hold tags.
% -----------------------------------------------------------------------------
\setstocksize{9.61in}{6.69in}
\settrimmedsize{9.61in}{6.69in}{*}
\settypeblocksize{7.45in}{4.15in}{*}     % ~66-72 char measure
\setlrmargins{0.92in}{*}{*}              % inner margin fixed; outer takes the slack
\setulmargins{1.05in}{*}{*}
\setmarginnotes{0.26in}{1.42in}{0.18in}  % sep / width / push -- the tag gutter
\setheaderspaces{*}{0.55in}{*}
\checkandfixthelayout

% -----------------------------------------------------------------------------
% Part / section numbering:  Parts run 0, I, II, III; sections reset per part
% as <part>.<n> (e.g. I.3).  The Coda is an unnumbered \part*.
% -----------------------------------------------------------------------------
\renewcommand\thepart{\ifcase\value{part}0\or I\or II\or III\else\Roman{part}\fi}
\counterwithin{section}{part}            % \thesection -> \thepart.\arabic{section}
\setsecnumdepth{section}
\settocdepth{section}

% -----------------------------------------------------------------------------
% Display headings (monochrome).  Parts: a centred display page closed by a
% heavy-over-thin double rule (the "authority" cue formerly carried by gold).
% Sections: small caps.  No colour anywhere.
% -----------------------------------------------------------------------------
% Part appearance (centred small caps + double rule beneath the title)
\renewcommand*{\parttitlefont}{\normalfont\centering\LARGE\scshape}
\renewcommand*{\partnamefont}{\normalfont\centering\large\scshape}
\renewcommand*{\partnumfont}{\normalfont\large\scshape}
\renewcommand*{\printparttitle}[1]{#1\par\vskip10pt
  \centerline{\rule{2.1in}{1.05pt}}\vskip2.4pt\centerline{\rule{2.1in}{0.4pt}}}

% Section appearance: small caps
\setsecheadstyle{\normalfont\large\scshape\raggedright}

% bold run-in leads ("\runin{The structural choice}") replace numbered subsections
\newcommand{\runin}[1]{\par\medskip\noindent\textbf{#1}\hskip0.6em\ignorespaces}

% -----------------------------------------------------------------------------
% Running heads (memoir page style).  Outer = page number, inner = section
% mark.  Thin 0.4pt black rule.  Part pages are clean (empty style).
% -----------------------------------------------------------------------------
\nouppercaseheads
\makepagestyle{ftd}
\makeoddhead{ftd}{\small\scshape\rightmark}{}{\small\thepage}
\makeevenhead{ftd}{\small\thepage}{}{\small\scshape\rightmark}
\makeoddfoot{ftd}{}{}{}
\makeevenfoot{ftd}{}{}{}
\makeheadrule{ftd}{\textwidth}{0.4pt}
\makepsmarks{ftd}{%
  \createmark{section}{right}{shownumber}{}{\quad}%
}
\aliaspagestyle{part}{empty}
\pagestyle{ftd}

% -----------------------------------------------------------------------------
% Theorem environments (amsthm, numbered within part.section scope).
% -----------------------------------------------------------------------------
\theoremstyle{plain}
\newtheorem{theorem}{Theorem}[section]
\newtheorem{lemma}[theorem]{Lemma}
\newtheorem{corollary}[theorem]{Corollary}
\newtheorem{proposition}[theorem]{Proposition}
\newtheorem{conjecture}[theorem]{Conjecture}
\theoremstyle{definition}
\newtheorem{definition}[theorem]{Definition}
\newtheorem{example}[theorem]{Example}
\theoremstyle{remark}
\newtheorem*{remark}{Remark}
\newtheorem*{observation}{Observation}
\renewcommand{\qedsymbol}{$\square$}

% -----------------------------------------------------------------------------
% Captions (small italic, bold label, period separator) -- monochrome.
% -----------------------------------------------------------------------------
\captiondelim{.\space}
\captionnamefont{\small\bfseries}
\captiontitlefont{\small\itshape}
\setlength{\belowcaptionskip}{2pt}

% -----------------------------------------------------------------------------
% Micro-typography & page-breaking discipline.
% -----------------------------------------------------------------------------
\RequirePackage{needspace}
\widowpenalty=10000
\clubpenalty=10000
\hyphenpenalty=400
\tolerance=1200
\setlength{\emergencystretch}{2.5em} % absorb long unbreakable lines / wide badges
\setlength{\parindent}{1.4em}
% Make escaped underscores in \texttt file names breakable (AUDIT_FOO_BAR.md),
% so long canonical-source paths no longer overrun the narrow measure.
\renewcommand{\_}{\textunderscore\allowbreak}
\setlength{\parskip}{0pt plus 1pt}
\raggedbottom
% Float placement: let a tall figure (the construction DAG, ~5.5in incl. caption)
% be admissible as a TOP float beneath its short section, instead of being exiled
% to its own page and leaving an orphan text remnant.
\renewcommand{\topfraction}{0.9}
\renewcommand{\floatpagefraction}{0.8}
\renewcommand{\textfraction}{0.08}
\setcounter{topnumber}{2}

% lemniscate section ornament (monochrome) -- used between major movements
\newcommand{\lemniscate}{%
  \par\vspace{1.1em}\begin{center}\large$\infty$\end{center}\vspace{1.1em}\par}

% -----------------------------------------------------------------------------
% Hyperref (print-safe: hidden links; § numbering already signals references).
% Loaded late; cleveref last.
% -----------------------------------------------------------------------------
\RequirePackage{hyperref}
\hypersetup{
  hidelinks,
  plainpages=false,
  bookmarksnumbered=true,
  bookmarksopen=true,
  bookmarksopenlevel=1,
  pdfstartview={FitH},
  pdftitle={The Construction of the FTD Mathematics},
  pdfsubject={From a discrete ontology to its provable boundary},
  pdfkeywords={Foundational Ternary Dynamics, discrete ontology, lemniscatic constant,
    master quadratic, fine-structure constant, route-invariant boundary, epistemic tagging},
}
% Give hyperref raw-arabic anchor names so the \ifcase part labels never
% collide (otherwise two sections can resolve to the same PDF destination).
\def\theHpart{part.\arabic{part}}
\def\theHsection{\theHpart.\arabic{section}}
% Bibliography: biblatex + biber.  Loaded here (after hyperref, before cleveref)
% so cleveref is genuinely last.  The master file supplies \addbibresource.
\RequirePackage[backend=biber, style=numeric-comp, sorting=nyt,
  maxbibnames=6, giveninits=true, useprefix=true]{biblatex}
\RequirePackage[nameinlink]{cleveref}
\crefname{section}{\S}{\S\S}
\Crefname{section}{Section}{Sections}
% reference helper matching the source's "§I.3" convention
\newcommand{\secref}[1]{\S\ref{#1}}

\endinput
