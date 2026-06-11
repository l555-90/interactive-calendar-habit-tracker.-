#!/bin/bash
# This is the exact command that worked in your terminal
powershell.exe -Command "\$path = Join-Path (Get-Location).Path 'buzzer.wav'; \$p = New-Object Media.SoundPlayer \$path; \$p.PlaySync();"