@echo off
echo [FIX] Refreshing Environment Variables and Git configuration...

:: Update User PATH with Git if not already there (moved to front)
powershell -Command "$oldPath = [Environment]::GetEnvironmentVariable('Path', 'User'); $cleanPath = $oldPath -replace 'C:\\Program Files\\Git\\bin;C:\\Program Files\\Git\\cmd', ''; $cleanPath = $cleanPath -replace ';;', ';'; $newPath = 'C:\Program Files\Git\bin;C:\Program Files\Git\cmd;' + $cleanPath; [Environment]::SetEnvironmentVariable('Path', $newPath, 'User')"

:: Notify Windows that environment changed (broadcast)
powershell -Command "$SendMessage = [Runtime.InteropServices.Marshal]::GetDelegateForFunctionPointer((Add-Type -PassThru 'using System; using System.Runtime.InteropServices; public class Win32 { [DllImport(\"user32.dll\", SetLastError = true, CharSet = CharSet.Auto)] public static extern IntPtr SendMessageTimeout(IntPtr hWnd, uint Msg, UIntPtr wParam, string lParam, uint fuFlags, uint uTimeout, out UIntPtr lpdwResult); }'::Win32).GetMethod('SendMessageTimeout').MethodHandle.GetFunctionPointer(), [Action[IntPtr, uint, UIntPtr, string, uint, uint, UIntPtr]]) ; $SendMessage.Invoke([IntPtr]0xffff, 0x001A, [UIntPtr]0, 'Environment', 2, 5000, [UIntPtr]0)"

echo [SUCCESS] Git has been moved to the front of your User PATH.
echo.
echo IMPORTANT: 
echo 1. You MUST RESTART CLion (close and reopen) for this to take effect.
echo 2. If it still doesn't work, try restarting your computer.
echo.
pause
