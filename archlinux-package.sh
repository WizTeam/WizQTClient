# compile archlinux package
cd ./build/arch/
sudo pacman -S --needed qt5-base qt5-webengine qt5-websockets qt5-tools cmake xdg-utils
makepkg
#install
#sudo pacman -U ./wiznote-2.3.3-1-x86_64.pkg.tar.xz
