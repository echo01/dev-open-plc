# How to Setup and Run OpenPLC Editor

This project is OpenPLC Editor `4.2.8`.

## 1. Required Node.js Version

Use Node.js `22.x` or `23.x`.

The project does not support Node.js `24.x` yet because `package.json` requires:

```json
"engines": {
  "node": ">=22.x <24",
  "npm": ">=10.x"
}
```

Recommended version:

```powershell
node -v
npm -v
```

Expected:

```text
node v22.x.x
npm 10.x or newer
```

If you use Windows and have `nvm-windows` installed:

```powershell
nvm install 22
nvm use 22
node -v
npm -v
```

## 2. PowerShell Note

On some Windows machines, PowerShell blocks `npm.ps1` and shows:

```text
npm.ps1 cannot be loaded because running scripts is disabled on this system
```

Use `npm.cmd` instead:

```powershell
npm.cmd install
npm.cmd run build
npm.cmd run dev
```

## 3. Install Dependencies

From the project root:

```powershell
cd D:\Product\OpenPLC\openplc-editor-development
npm.cmd install
```

During install, the project runs `postinstall`, which downloads and installs:

- `strucpp v0.5.13`
- `xml2st v4.0.7`
- Electron native dependencies
- renderer development DLL

Successful install should include output similar to:

```text
added xxxx packages
strucpp v0.5.13 installed.
xml2st v4.0.7 installed.
completed installing native dependencies
```

Warnings such as deprecated packages or Browserslist warnings do not necessarily mean the install failed.

## 4. Optional: Run Audit Fix Carefully

You may run:

```powershell
npm.cmd audit fix
```

Do not run this unless you are ready to test the build afterward.

Avoid this unless you intentionally want breaking dependency upgrades:

```powershell
npm.cmd audit fix --force
```

`--force` may upgrade major build dependencies such as Electron, Monaco Editor, Webpack Dev Server, or XML libraries and can break the project.

## 5. If Build Cannot Resolve `strucpp`

After `npm audit fix`, `strucpp` may be removed from `node_modules` because it is installed by the project script with `--no-save`.

Typical build errors:

```text
Can't resolve 'strucpp'
Can't resolve 'strucpp/libs/iec-types.json'
Can't resolve 'strucpp/dist/browser-server.js?url'
```

Fix:

```powershell
npm.cmd run setup:strucpp
```

If the command fails with a certificate error:

```text
UNABLE_TO_VERIFY_LEAF_SIGNATURE
```

This is usually caused by local proxy or SSL inspection. A temporary workaround for this command only is:

```cmd
cmd.exe /c "set NODE_TLS_REJECT_UNAUTHORIZED=0&& npm.cmd run setup:strucpp"
```

Use this only as a temporary workaround. The better long-term fix is to configure the correct corporate/root CA certificate for Node.js.

You can verify the required files exist:

```powershell
Test-Path node_modules\strucpp
Test-Path node_modules\strucpp\libs\iec-types.json
Test-Path node_modules\strucpp\dist\browser-server.js
```

All should return:

```text
True
```

## 6. Build the Project

Run:

```powershell
npm.cmd run build
```

This runs:

```text
npm run build:main
npm run build:renderer
```

Successful build should end with:

```text
npm run build:main exited with code 0
npm run build:renderer exited with code 0
```

`npm.cmd run build` compiles the Electron main process and React renderer for production. It does not open the application.

## 7. Run in Development Mode

Run:

```powershell
npm.cmd run dev
```

This will:

- check port `1313`
- run `prestart`
- download missing binaries if needed
- build the Electron main process in development mode
- start the renderer webpack dev server
- launch Electron via `electronmon`

Use this command when you want to open and test the OpenPLC Editor app during development.

## 8. Installed Package Location

OpenPLC Editor installed VPP packages are stored under Electron `userData`:

```text
C:\Users\User\AppData\Roaming\open-plc-editor\packages
```

The package registry file is:

```text
C:\Users\User\AppData\Roaming\open-plc-editor\packages\registry.json
```

Example installed package folders:

```text
C:\Users\User\AppData\Roaming\open-plc-editor\packages\com.openplc.stm32-community
C:\Users\User\AppData\Roaming\open-plc-editor\packages\com.openplc.raspberry-pi
C:\Users\User\AppData\Roaming\open-plc-editor\packages\com.openplc.espressif
```

## 9. Common Command Summary

```powershell
cd D:\Product\OpenPLC\openplc-editor-development

node -v
npm -v

npm.cmd install
npm.cmd audit fix
npm.cmd run setup:strucpp
npm.cmd run build
npm.cmd run dev
```

