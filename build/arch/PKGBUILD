# Maintainer: Albert.Zhou <albert.zhou@wiz.cn>
pkgname=wiznote
pkgver=2.0.56
pkgrel=1
pkgdesc="Opensource cross-platform cloud based note-taking client"
arch=('i686' 'x86_64')
url="http://www.wiznote.com"
license=('GPL3' 'custom')
depends=('desktop-file-utils' 'hicolor-icon-theme' 'xdg-utils' 'qt5-base' 'qt5-webkit') # indirect: qt5-declarative qt5-sensors zlib glic gcc-libs
makedepends=('cmake>=2.8.9' 'qt5-tools')
install=wiznote.install
#source=("$pkgname::git+https://github.com/WizTeam/WizQTClient.git#tag=v2.0.38") # Use git is too slow
#md5sums=('SKIP')
wiznote_project_name="WizQTClient"
source=("$pkgname.tar.gz::https://github.com/WizTeam/WizQTClient/archive/v2.0.56.tar.gz")
md5sums=('52c59870cee5691d3f50c0f84344714e')

build() {
	cd "$srcdir/$wiznote_project_name-$pkgver"
    cmake -DWIZNOTE_USE_QT5=YES -DCMAKE_INSTALL_PREFIX=/usr/ .
	make
}

package() {
	cd "$srcdir/$wiznote_project_name-$pkgver"
	make DESTDIR="$pkgdir/" install
}
