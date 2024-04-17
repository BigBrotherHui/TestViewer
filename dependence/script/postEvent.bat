set SolutionDir=%1
set SolutionName=%2
set ProjectDir=%3
set ProjectName=%4
set Platform=%5
set Configuration=%6
set TargetExt=%7
set TargetDirectory=%8

if "%Configuration%"=="Debug" (
    xcopy "%SolutionDir%\lib\%Platform%\%Configuration%\%ProjectName%.pdb" "%SolutionDir%\bin\%Platform%\%Configuration%\%TargetDirectory%\" /Y
) 

xcopy "%SolutionDir%\lib\%Platform%\%Configuration%\%ProjectName%%TargetExt%" "%SolutionDir%\bin\%Platform%\%Configuration%\%TargetDirectory%\" /Y
if errorlevel 1 (
    echo failed
) else (
    echo success
)
if "%Configuration%"=="Debug" (
    xcopy "%SolutionDir%\lib\%Platform%\%Configuration%\%ProjectName%.pdb" "%SolutionDir%\bin\%Platform%\%Configuration%\%TargetDirectory%\" /Y
)

pause
