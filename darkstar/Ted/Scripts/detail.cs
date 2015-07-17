alias setDT "set SimTerrain::DetailTable"
alias setVD "set SimTerrain::VisibleDistance"
alias setPD "set SimTerrain::PerspectiveDistance"
alias setHD "set SimTerrain::HazeDistance"

alias dtNear "setDT 750 1500 3000 3500 6000 7000 10000 12000 18000 20000 30000 50000 50000 300000"
alias visNear "dtNear;setPD 1000;setVD 10000;setHD 5000"
alias visMid "dtNear;setPD 1000;setVD 25000;setHD 15000"
alias visFar "dtNear;setPD 1000;setVD 150000;setHD 60000"