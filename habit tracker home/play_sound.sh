#!/bin/bash
# Using a different method to trigger Windows audio
powershell.exe -c "Add-Type -AssemblyName PresentationCore; \$mediaPlayer = New-Object System.Windows.Media.MediaPlayer; \$mediaPlayer.Open('C:/Users/User I55-90/OneDrive/Desktop/vscode/habit tracker calander/buzzer.wav'); \$mediaPlayer.Play(); Start-Sleep -Seconds 2;"