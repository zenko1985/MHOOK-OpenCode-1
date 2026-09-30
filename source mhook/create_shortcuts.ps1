$ws = New-Object -ComObject WScript.Shell
$s = $ws.CreateShortcut([Environment]::GetFolderPath('Desktop') + '\mhook.lnk')
$s.TargetPath = 'C:\Programs\mhook\source mhook\MHook64.sln'
$s.WorkingDirectory = 'C:\Programs\mhook\source mhook'
$s.Save()

$s2 = $ws.CreateShortcut([Environment]::GetFolderPath('Desktop') + '\mishkinamish.lnk')
$s2.TargetPath = 'C:\Programs\mhook\source mishkinamish\mishkinamish.sln'
$s2.WorkingDirectory = 'C:\Programs\mhook\source mishkinamish'
$s2.Save()
