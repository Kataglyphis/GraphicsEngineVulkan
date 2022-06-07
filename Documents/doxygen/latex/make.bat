set Dir_Old=%cd%
cd /D %~dp0


set ORG_LATEX_CMD=%LATEX_CMD%
set ORG_MKIDX_CMD=%MKIDX_CMD%
set ORG_BIBTEX_CMD=%BIBTEX_CMD%
set ORG_LATEX_COUNT=%LATEX_COUNT%
set ORG_MANUAL_FILE=%MANUAL_FILE%
if "X"%LATEX_CMD% == "X" set LATEX_CMD=latex
if "X"%MKIDX_CMD% == "X" set MKIDX_CMD=makeindex
if "X"%BIBTEX_CMD% == "X" set BIBTEX_CMD=bibtex
if "X"%LATEX_COUNT% == "X" set LATEX_COUNT=8
if "X"%MANUAL_FILE% == "X" set MANUAL_FILE=refman

del /s /f *.ps *.dvi *.aux *.toc *.idx *.ind *.ilg *.log *.out *.brf *.blg *.bbl %MANUAL_FILE%.pdf


%LATEX_CMD% %MANUAL_FILE%.tex
echo ----
%MKIDX_CMD% %MANUAL_FILE%.idx
setlocal enabledelayedexpansion
set count=%LAT#EX_COUNT%
:repeat
set content=X
for /F "tokens=*" %%T in ( 'findstr /C:"Rerun LaTeX" %MANUAL_FILE%.log' ) do set content="%%~T"
if !content! == X for /F "tokens=*" %%T in ( 'findstr /C:"Rerun to get cross-references right" %MANUAL_FILE%.log' ) do set content="%%~T"
if !content! == X for /F "tokens=*" %%T in ( 'findstr /C:"Rerun to get bibliographical references right" %MANUAL_FILE%.log' ) do set content="%%~T"
if !content! == X goto :skip
set /a count-=1
if !count! EQU 0 goto :skip

echo ----
%LATEX_CMD% %MANUAL_FILE%.tex
goto :repeat
:skip
endlocal
%MKIDX_CMD% %MANUAL_FILE%.idx
%LATEX_CMD% %MANUAL_FILE%.tex
dvips -o %MANUAL_FILE%.ps %MANUAL_FILE%.dvi
gswin32c.exe -q -dNOPAUSE -dBATCH -sDEVICE=pdfwrite -sOutputFile=%MANUAL_FILE%.pdf -c save pop -f %MANUAL_FILE%.ps

@REM reset environment
cd /D %Dir_Old%
set Dir_Old=
set LATEX_CMD=%ORG_LATEX_CMD%
set ORG_LATEX_CMD=
set MKIDX_CMD=%ORG_MKIDX_CMD%
set ORG_MKIDX_CMD=
set BIBTEX_CMD=%ORG_BIBTEX_CMD%
set ORG_BIBTEX_CMD=
set MANUAL_FILE=%ORG_MANUAL_FILE%
set ORG_MANUAL_FILE=
set LATEX_COUNT=%ORG_LATEX_COUNT%
set ORG_LATEX_COUNT=
