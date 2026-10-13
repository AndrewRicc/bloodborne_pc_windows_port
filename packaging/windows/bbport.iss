; Inno Setup script of the Windows installer. Built by scripts\windows\make_installer.ps1, which
; stages the files (never any game file, never the game's icon) and passes the defines below.
; The installer checks the PC, asks where the user's own game copy is, and installs the port.

#ifndef AppVersion
  #define AppVersion "dev"
#endif
#ifndef Stage
  #define Stage "..\..\dist\stage"
#endif
#ifndef HasDlss
  #define HasDlss "0"
#endif

[Setup]
AppId={{6F2B9C41-3D7A-4E58-9B1C-2A7D5E8F0C13}
AppName=Bloodborne PC Port
AppVersion={#AppVersion}
AppPublisher=bbport contributors
AppPublisherURL=https://github.com/AndrewRicc/bloodborne_pc_windows_port
; The port writes its settings, saves and prepared game files next to itself: the folder must
; be writable without administrator rights.
DefaultDirName={localappdata}\Programs\bbport
DefaultGroupName=Bloodborne PC Port
DisableProgramGroupPage=yes
PrivilegesRequired=lowest
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
WizardStyle=modern
LicenseFile={#Stage}\LICENSE
InfoBeforeFile={#Stage}\README_RELEASE.txt
OutputDir=..\..\dist
OutputBaseFilename=bbport-setup-{#AppVersion}
Compression=lzma2/max
SolidCompression=yes
UninstallDisplayName=Bloodborne PC Port
SetupLogging=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "Create a &desktop shortcut"; GroupDescription: "Shortcuts:"
#if HasDlss == "0"
; Nothing of NVIDIA is in this package: DLSS is built on the user's PC by build_dlss.ps1.
Name: "dlss"; Description: "Build &DLSS on this PC: downloads NVIDIA's SDK files (63 MB) and, if missing, the Microsoft C++ compiler (about 7 GB)"; GroupDescription: "NVIDIA DLSS (can also be done later with build_dlss.bat):"; Flags: unchecked; Check: HasRtx
#endif

[Files]
Source: "{#Stage}\*"; DestDir: "{app}"; Excludes: "\scripts\windows\make_icon.ps1"; Flags: recursesubdirs createallsubdirs ignoreversion
; Last file: once it is in place the settings and the icon are written, before the shortcuts.
Source: "{#Stage}\scripts\windows\make_icon.ps1"; DestDir: "{app}\scripts\windows"; Flags: ignoreversion; AfterInstall: AfterFilesInstalled
Source: "check_system.ps1"; Flags: dontcopy

[Icons]
Name: "{group}\Bloodborne PC Port"; Filename: "{app}\Bloodborne.exe"; WorkingDir: "{app}"; IconFilename: "{code:ShortcutIcon}"
Name: "{group}\Uninstall Bloodborne PC Port"; Filename: "{uninstallexe}"
Name: "{autodesktop}\Bloodborne PC Port"; Filename: "{app}\Bloodborne.exe"; WorkingDir: "{app}"; IconFilename: "{code:ShortcutIcon}"; Tasks: desktopicon

[Run]
#if HasDlss == "0"
Filename: "{sys}\WindowsPowerShell\v1.0\powershell.exe"; Parameters: "-NoProfile -ExecutionPolicy Bypass -File ""{app}\scripts\windows\build_dlss.ps1"" -Yes -Pause"; WorkingDir: "{app}"; StatusMsg: "Building DLSS: see the console window..."; Tasks: dlss; Flags: skipifsilent
#endif
Filename: "{app}\Bloodborne.exe"; Description: "Open the launcher now"; WorkingDir: "{app}"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
; Files the port makes while running. out\ holds files prepared from the user's game copy.
; Saves (user\) and settings (bbport.ini, launcher_settings.json) are left in place.
Type: filesandordirs; Name: "{app}\out"
; NVIDIA's SDK files downloaded by build_dlss.ps1.
Type: filesandordirs; Name: "{app}\modules\dlss-sdk"
Type: dirifempty; Name: "{app}\modules"
Type: filesandordirs; Name: "{app}\scripts\__pycache__"
Type: files; Name: "{app}\bloodborne.ico"
Type: files; Name: "{app}\python_path.txt"
Type: files; Name: "{app}\launcher.log"

[Code]
var
  CheckPage: TWizardPage;
  CheckMemo: TNewMemo;
  CheckNote: TNewStaticText;
  RecheckButton, PythonButton: TNewButton;
  GamePage: TWizardPage;
  GameEdit: TNewEdit;
  GameBrowse: TNewButton;
  GameText, GameLabel: TNewStaticText;
  SavesPage: TWizardPage;
  SavesEdit: TNewEdit;
  SavesBrowse: TNewButton;
  SavesText, SavesLabel: TNewStaticText;
  CheckDone: Boolean;
  MissingCount: Integer;
  PythonMissing, RtxFound: Boolean;
  PythonPath, GameDir, SavesDir, SavesResult: String;

function PowerShellExe: String;
begin
  Result := ExpandConstant('{sys}\WindowsPowerShell\v1.0\powershell.exe');
end;

procedure RunCheck;
var
  ResultCode, I, Bar: Integer;
  Script, OutFile, State, Text, Shown: String;
  Lines: TArrayOfString;
begin
  CheckMemo.Text := 'Checking this PC...';
  WizardForm.Refresh;
  MissingCount := 0;
  PythonMissing := False;
  RtxFound := False;
  PythonPath := '';
  ExtractTemporaryFile('check_system.ps1');
  Script := ExpandConstant('{tmp}\check_system.ps1');
  OutFile := ExpandConstant('{tmp}\check_system.txt');
  DeleteFile(OutFile);
  Exec(PowerShellExe, '-NoProfile -ExecutionPolicy Bypass -File "' + Script + '" -Out "' + OutFile +
       '" -HasDlss {#HasDlss}', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
  Shown := '';
  if LoadStringsFromFile(OutFile, Lines) then begin
    for I := 0 to GetArrayLength(Lines) - 1 do begin
      Bar := Pos('|', Lines[I]);
      if Bar = 0 then Continue;
      State := Copy(Lines[I], 1, Bar - 1);
      Text := Copy(Lines[I], Bar + 1, Length(Lines[I]));
      if State = 'PYTHON' then
        PythonPath := Text
      else if State = 'RTX' then
        RtxFound := True
      else begin
        if State = 'MISSING' then begin
          MissingCount := MissingCount + 1;
          if Pos('Python', Text) > 0 then PythonMissing := True;
          Shown := Shown + '[MISSING]  ' + Text + #13#10;
        end else if State = 'WARN' then
          Shown := Shown + '[NOTE]     ' + Text + #13#10
        else
          Shown := Shown + '[OK]       ' + Text + #13#10;
      end;
    end;
  end else begin
    MissingCount := 1;
    Shown := '[MISSING]  The system check could not run (Windows PowerShell is required).' + #13#10;
  end;
  CheckMemo.Text := Shown;
  PythonButton.Visible := PythonMissing;
  if MissingCount = 0 then
    CheckNote.Caption := 'This PC can run the port. Click Next to continue.'
  else if PythonMissing and (MissingCount = 1) then
    CheckNote.Caption := 'Python is missing. Click "Install Python", then "Check again".'
  else
    CheckNote.Caption := 'The items marked [MISSING] must be fixed before installing.';
  CheckDone := True;
end;

procedure RecheckClick(Sender: TObject);
begin
  RunCheck;
end;

procedure PythonClick(Sender: TObject);
var
  ResultCode: Integer;
begin
  if SuppressibleMsgBox('Python 3.13 will be downloaded and installed with winget (about 30 MB, from python.org).' + #13#10 +
            'A console window shows the progress. Continue?', mbConfirmation, MB_YESNO, IDYES) <> IDYES then Exit;
  if not Exec(ExpandConstant('{cmd}'), '/c winget install --id Python.Python.3.13 -e', '', SW_SHOW,
              ewWaitUntilTerminated, ResultCode) then
    SuppressibleMsgBox('winget could not be started. Install Python 3.10 or newer from python.org with the default options, then click "Check again".',
           mbError, MB_OK, IDOK);
  RunCheck;
end;

// The DLSS task is offered only on a PC with an NVIDIA RTX card (found by the system check).
function HasRtx: Boolean;
begin
  Result := RtxFound;
end;

function StripGamePath(Path: String): String;
begin
  Result := Trim(RemoveQuotes(Trim(Path)));
  if CompareText(ExtractFileName(Result), 'eboot.bin') = 0 then Result := ExtractFileDir(Result);
  Result := RemoveBackslashUnlessRoot(Result);
end;

function JsonEscape(Text: String): String;
begin
  Result := Text;
  StringChangeEx(Result, '\', '\\', True);
  StringChangeEx(Result, '"', '\"', True);
end;

procedure GameBrowseClick(Sender: TObject);
var
  Dir: String;
begin
  Dir := StripGamePath(GameEdit.Text);
  if BrowseForFolder('Select your Bloodborne folder (the one with eboot.bin):', Dir, False) then GameEdit.Text := Dir;
end;

procedure SavesBrowseClick(Sender: TObject);
var
  Dir: String;
begin
  Dir := RemoveBackslashUnlessRoot(Trim(RemoveQuotes(Trim(SavesEdit.Text))));
  if BrowseForFolder('Select the old installation folder, or its user\savedata folder:', Dir, False) then SavesEdit.Text := Dir;
end;

// True when the folder holds save data of the port: savedata\<user id>\<title id>\, looked for in
// the folder itself and in the usual places below an installation folder.
function HasSaves(Dir: String): Boolean;
var
  Roots: array[0..4] of String;
  I: Integer;
  Users, Titles: TFindRec;
begin
  Result := False;
  Roots[0] := Dir + '\user\savedata';
  Roots[1] := Dir + '\savedata';
  Roots[2] := Dir;
  Roots[3] := ExtractFileDir(Dir);
  Roots[4] := ExtractFileDir(ExtractFileDir(Dir));
  for I := 0 to 4 do begin
    if FindFirst(Roots[I] + '\*', Users) then begin
      try
        repeat
          if (Users.Attributes and FILE_ATTRIBUTE_DIRECTORY <> 0) and (Users.Name <> '.') and (Users.Name <> '..') then begin
            if FindFirst(Roots[I] + '\' + Users.Name + '\CUSA*', Titles) then begin
              FindClose(Titles);
              Result := True;
            end;
          end;
        until Result or not FindNext(Users);
      finally
        FindClose(Users);
      end;
    end;
    if Result then Exit;
  end;
end;

procedure InitializeWizard;
begin
  CheckPage := CreateCustomPage(wpInfoBefore, 'System check',
    'The installer checks that this PC can run the port.');
  CheckMemo := TNewMemo.Create(CheckPage);
  CheckMemo.Parent := CheckPage.Surface;
  CheckMemo.SetBounds(0, 0, CheckPage.SurfaceWidth, CheckPage.SurfaceHeight - ScaleY(64));
  CheckMemo.ReadOnly := True;
  CheckMemo.ScrollBars := ssVertical;
  CheckMemo.WordWrap := True;
  CheckMemo.Font.Name := 'Consolas';
  CheckNote := TNewStaticText.Create(CheckPage);
  CheckNote.Parent := CheckPage.Surface;
  CheckNote.SetBounds(0, CheckPage.SurfaceHeight - ScaleY(56), CheckPage.SurfaceWidth, ScaleY(18));
  RecheckButton := TNewButton.Create(CheckPage);
  RecheckButton.Parent := CheckPage.Surface;
  RecheckButton.SetBounds(0, CheckPage.SurfaceHeight - ScaleY(30), ScaleX(110), ScaleY(26));
  RecheckButton.Caption := 'Check again';
  RecheckButton.OnClick := @RecheckClick;
  PythonButton := TNewButton.Create(CheckPage);
  PythonButton.Parent := CheckPage.Surface;
  PythonButton.SetBounds(ScaleX(120), CheckPage.SurfaceHeight - ScaleY(30), ScaleX(130), ScaleY(26));
  PythonButton.Caption := 'Install Python';
  PythonButton.OnClick := @PythonClick;
  PythonButton.Visible := False;

  // A custom page and not CreateInputDirPage: that one refuses an empty folder, and the game
  // folder may be chosen later in the launcher.
  GamePage := CreateCustomPage(wpSelectDir, 'Your copy of Bloodborne', 'Where is your game folder?');
  GameText := TNewStaticText.Create(GamePage);
  GameText.Parent := GamePage.Surface;
  GameText.AutoSize := False;
  GameText.WordWrap := True;
  GameText.SetBounds(0, 0, GamePage.SurfaceWidth, ScaleY(96));
  GameText.Caption :=
    'The port needs your own copy of Bloodborne (CUSA03173, version 1.09), dumped from your PS4: the folder ' +
    'that contains eboot.bin, dvdroot_ps4 and sce_sys. The game is not included and is not copied: the port ' +
    'reads it where it is.' + #13#10#13#10 +
    'Pick the folder, then click Next. You can leave it empty and choose it later in the launcher.';
  GameLabel := TNewStaticText.Create(GamePage);
  GameLabel.Parent := GamePage.Surface;
  GameLabel.SetBounds(0, ScaleY(104), GamePage.SurfaceWidth, ScaleY(16));
  GameLabel.Caption := 'Game folder (the one with eboot.bin):';
  GameEdit := TNewEdit.Create(GamePage);
  GameEdit.Parent := GamePage.Surface;
  GameEdit.SetBounds(0, ScaleY(124), GamePage.SurfaceWidth - ScaleX(96), ScaleY(23));
  // Silent installs: setup.exe /VERYSILENT /SUPPRESSMSGBOXES /GAME="D:\path\CUSA03173"
  GameEdit.Text := ExpandConstant('{param:GAME|}');
  GameBrowse := TNewButton.Create(GamePage);
  GameBrowse.Parent := GamePage.Surface;
  GameBrowse.SetBounds(GamePage.SurfaceWidth - ScaleX(88), ScaleY(123), ScaleX(88), ScaleY(25));
  GameBrowse.Caption := 'Browse...';
  GameBrowse.OnClick := @GameBrowseClick;

  SavesPage := CreateCustomPage(GamePage.ID, 'Your save data', 'Do you have saves from an earlier installation?');
  SavesText := TNewStaticText.Create(SavesPage);
  SavesText.Parent := SavesPage.Surface;
  SavesText.AutoSize := False;
  SavesText.WordWrap := True;
  SavesText.SetBounds(0, 0, SavesPage.SurfaceWidth, ScaleY(96));
  SavesText.Caption :=
    'To keep playing from where you were, pick the folder of your earlier installation of the port, or just ' +
    'its user\savedata folder. The saves are copied into this installation; the originals are not touched.' + #13#10#13#10 +
    'Leave it empty to start without importing. You can also do it later with "Import saves..." in the launcher.';
  SavesLabel := TNewStaticText.Create(SavesPage);
  SavesLabel.Parent := SavesPage.Surface;
  SavesLabel.SetBounds(0, ScaleY(104), SavesPage.SurfaceWidth, ScaleY(16));
  SavesLabel.Caption := 'Folder with your saves (optional):';
  SavesEdit := TNewEdit.Create(SavesPage);
  SavesEdit.Parent := SavesPage.Surface;
  SavesEdit.SetBounds(0, ScaleY(124), SavesPage.SurfaceWidth - ScaleX(96), ScaleY(23));
  // Silent installs: /SAVES="D:\old\bbport"
  SavesEdit.Text := ExpandConstant('{param:SAVES|}');
  SavesBrowse := TNewButton.Create(SavesPage);
  SavesBrowse.Parent := SavesPage.Surface;
  SavesBrowse.SetBounds(SavesPage.SurfaceWidth - ScaleX(88), ScaleY(123), ScaleX(88), ScaleY(25));
  SavesBrowse.Caption := 'Browse...';
  SavesBrowse.OnClick := @SavesBrowseClick;
end;

procedure CurPageChanged(CurPageID: Integer);
begin
  if (CurPageID = CheckPage.ID) and not CheckDone then RunCheck;
end;

function NextButtonClick(CurPageID: Integer): Boolean;
var
  Dir: String;
  Sfo: AnsiString;
begin
  Result := True;
  if CurPageID = CheckPage.ID then begin
    if MissingCount > 0 then begin
      SuppressibleMsgBox('Some requirements are missing. Fix the items marked [MISSING], then click "Check again".', mbError, MB_OK, IDOK);
      Result := False;
    end;
  end else if CurPageID = GamePage.ID then begin
    Dir := StripGamePath(GameEdit.Text);
    GameDir := '';
    if Dir = '' then begin
      Result := SuppressibleMsgBox('No game folder chosen. The port is installed anyway; you will pick eboot.bin in the launcher, ' +
                       'and the shortcut gets a standard icon. Continue?', mbConfirmation, MB_YESNO, IDYES) = IDYES;
      Exit;
    end;
    if not FileExists(Dir + '\eboot.bin') or not DirExists(Dir + '\dvdroot_ps4') or
       not FileExists(Dir + '\sce_sys\param.sfo') then begin
      SuppressibleMsgBox('This is not a Bloodborne game folder: eboot.bin, dvdroot_ps4 and sce_sys\param.sfo must be in it.' + #13#10#13#10 + Dir,
             mbError, MB_OK, IDOK);
      Result := False;
      Exit;
    end;
    if LoadStringFromFile(Dir + '\sce_sys\param.sfo', Sfo) and
       ((Pos('CUSA03173', Sfo) = 0) or (Pos('01.09', Sfo) = 0)) then begin
      if SuppressibleMsgBox('This copy does not look like CUSA03173 version 1.09, the only one the port supports. ' +
                'It will probably not start. Use it anyway?', mbConfirmation, MB_YESNO, IDYES) <> IDYES then begin
        Result := False;
        Exit;
      end;
    end;
    GameDir := Dir;
    GameEdit.Text := Dir;
  end else if CurPageID = SavesPage.ID then begin
    Dir := RemoveBackslashUnlessRoot(Trim(RemoveQuotes(Trim(SavesEdit.Text))));
    SavesDir := '';
    if Dir = '' then Exit;
    if not DirExists(Dir) or not HasSaves(Dir) then begin
      SuppressibleMsgBox('No save data found in this folder:' + #13#10#13#10 + Dir + #13#10#13#10 +
                         'Pick the folder of the earlier installation or its user\savedata folder, or leave the field empty.',
                         mbError, MB_OK, IDOK);
      Result := False;
      Exit;
    end;
    SavesDir := Dir;
    SavesEdit.Text := Dir;
  end;
end;

// Copies the chosen saves with the port's own script (scripts\savedata_import.py): saves already
// in this installation are moved to user\savedata_backup_<date>, never overwritten.
procedure ImportSaves(App: String);
var
  ResultCode: Integer;
  OutFile: String;
  Output: AnsiString;
begin
  if SavesDir = '' then Exit;
  SavesResult := 'The saves could not be imported. Use "Import saves..." in the launcher.';
  if PythonPath = '' then Exit;
  OutFile := ExpandConstant('{tmp}\savedata_import.txt');
  if Exec(ExpandConstant('{cmd}'), '/c ""' + PythonPath + '" "' + App + '\scripts\savedata_import.py" "' + SavesDir +
          '" --user "' + App + '\user" > "' + OutFile + '" 2>&1"', App, SW_HIDE, ewWaitUntilTerminated, ResultCode) then begin
    if LoadStringFromFile(OutFile, Output) then Log('savedata_import: ' + Output);
    if ResultCode = 0 then SavesResult := 'Your saves were imported.'
    else if Output <> '' then SavesResult := 'The saves were not imported: ' + Trim(Output);
  end;
  if ResultCode <> 0 then SuppressibleMsgBox(SavesResult, mbError, MB_OK, IDOK);
end;

// Runs when the last file is in place, before the shortcuts are created.
procedure AfterFilesInstalled;
var
  App, Settings, Png: String;
  ResultCode: Integer;
  Json: TArrayOfString;
begin
  App := ExpandConstant('{app}');
  if PythonPath <> '' then SaveStringToFile(App + '\python_path.txt', PythonPath + #13#10, False);
  ImportSaves(App);
  if GameDir = '' then Exit;
  // An existing settings file (update or reinstall) keeps the user's launcher choices.
  Settings := App + '\launcher_settings.json';
  if not FileExists(Settings) then begin
    SetArrayLength(Json, 3);
    Json[0] := '{';
    Json[1] := '  "eboot": "' + JsonEscape(GameDir + '\eboot.bin') + '"';
    Json[2] := '}';
    SaveStringsToUTF8FileWithoutBOM(Settings, Json, False);
  end;
  // The shortcut icon is made here from the user's own game copy; it is not in this package.
  Png := GameDir + '\sce_sys\icon0.png';
  if FileExists(Png) then
    Exec(PowerShellExe, '-NoProfile -ExecutionPolicy Bypass -File "' + App + '\scripts\windows\make_icon.ps1" -Png "' +
         Png + '" -Out "' + App + '\bloodborne.ico"', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
end;

function ShortcutIcon(Param: String): String;
begin
  Result := ExpandConstant('{app}\bloodborne.ico');
  if not FileExists(Result) then Result := ExpandConstant('{app}\Bloodborne.exe');
end;

function UpdateReadyMemo(Space, NewLine, MemoUserInfoInfo, MemoDirInfo, MemoTypeInfo, MemoComponentsInfo,
  MemoGroupInfo, MemoTasksInfo: String): String;
begin
  Result := MemoDirInfo + NewLine + NewLine + 'Game folder:' + NewLine + Space;
  if GameDir <> '' then Result := Result + GameDir else Result := Result + '(to be chosen in the launcher)';
  Result := Result + NewLine + NewLine + 'Save data:' + NewLine + Space;
  if SavesDir <> '' then Result := Result + 'import from ' + SavesDir else Result := Result + '(nothing to import)';
  if MemoTasksInfo <> '' then Result := Result + NewLine + NewLine + MemoTasksInfo;
end;
