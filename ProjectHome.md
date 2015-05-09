## Introduction ##

This is a light-weight screen locker application designed for Windows 7 (and works on other systems as well).

When the desktop is idle for 1 minute (by default), the screen is locked and becomes completely black. Users cannot access the desktop before entering the correct password. It's impossible to stop the locker via shortcut keys including:
  * Alt + F4
  * Ctrl + Alt + Delete
  * Alt + Tab
  * Win + Tab
  * Alt + ESC
  * Ctrl + ESC

<font color='#e60012'><b>Note: The user is still able to press Ctrl + Alt + Delete and then restart the computer!</b></font>

## Commandline Help ##

<pre>  ScreenLock.exe [-h] [-i] [-s]<br>
-----------------------------<br>
-h: Print this help.<br>
-i: Lock screen immediately.<br>
-s: Secret mode (do not display tray icon).</pre>

## Change Logs ##

<font color='#3366ff'><b>Screen Locker 1.0.1 (released 2014-01-02)</b></font>

This is a maintainance release. The following items were added or fixed:

  * Add a option in the notification icon menu to stop auto lock temporarily
  * Add a option in the notification icon menu to set timeout
  * Fix bug: the notification icon menu does not disappear when user clicks outside of the menu
  * Fix bug: the screen is not completely covered in some circumstances

<a href='http://win32-netmon.googlecode.com/files/ScreenLock-1.0.1.zip'>Click here</a> to download.

<font color='#3366ff'><b>Screen Locker 1.0.0 (released 2013-12-31)</b></font>

This is the first release. The following features were included:

  * Global hot key Ctrl + Alt + H to lock immediately
  * SHA1 encryption for stored passwords

<a href='http://win32-netmon.googlecode.com/files/ScreenLock-1.0.0.zip'>Click here</a> to download.

## Background Information ##

In some particular circumstances, e.g., in the computer room of a university, we may want our application kept on top of all other windows, until the correct password is entered. Another example is that, we want to lock the desktop when it's idle for some time, however, we do not want the welcome screen to be displayed, and we do not wish to set a password for the current administrator.

You may come across some problems when designing such a program, among which the biggest one is that, when user pressed Ctrl + Alt + Delete, the welcome screen would show up immediately (in Windows XP, the task manager is started).

Although it's possible to disalbe Ctrl + Alt + Delete in Windows XP (with global low-level keyboard hook), it's not in Windows 7. Actually, you'll never be able to disable it without accessing the Windows kernel. A recommended approach is to write a keyboard filter driver and filter out the key strokes of interest. However, that's not our concern here.

A minor problem is that, when you write a screen locker application with password confirmation and set it as your screensaver, it just does not work although it seems to: Windows 7 would kill the screensaver process when Ctrl + Alt + Delete is pressed!

This program is intended to be a simple demo that just works around. Feel free to view the source code. It's rather simple, isn't it?

