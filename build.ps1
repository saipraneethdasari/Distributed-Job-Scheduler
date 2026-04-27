# Build script for mini-scheduler project

Write-Host "Building mini-scheduler..." -ForegroundColor Cyan

# Create build directory if it doesn't exist
if (-not (Test-Path "build")) {
    New-Item -ItemType Directory -Path "build" | Out-Null
}

# Build scheduler
Write-Host "`n[1/2] Building scheduler..." -ForegroundColor Yellow
g++ -std=c++17 -pthread scheduler/scheduler.cpp scheduler/server.cpp -lws2_32 -o build/scheduler.exe
if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Failed to build scheduler" -ForegroundColor Red
    exit 1
}
Write-Host "Scheduler built successfully: build/scheduler.exe" -ForegroundColor Green

# Build worker
Write-Host "`n[2/2] Building worker..." -ForegroundColor Yellow
g++ -std=c++17 -pthread worker/worker.cpp -lws2_32 -o build/worker.exe
if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Failed to build worker" -ForegroundColor Red
    exit 1
}
Write-Host "Worker built successfully: build/worker.exe" -ForegroundColor Green

Write-Host "`nAll builds completed successfully!" -ForegroundColor Green
Write-Host "`nExecutables:" -ForegroundColor Cyan
Get-ChildItem build -Filter *.exe | Select-Object Name
