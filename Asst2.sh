export PATH=$PATH:$HOME/os161/tools/bin

cd src/kern/conf
./config ASST2 
cd ../compile/ASST2
bmake depend 
bmake 
bmake install

cd ~/os161/root
ls
sys161 kernel-ASST2
