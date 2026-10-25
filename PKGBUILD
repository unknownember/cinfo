# Maintainer: kori2 nothankyou@gmail.com
pkgname=cinfo
pkgver=1.0.0
pkgrel=1
pkgdesc="A tiny, fast Linux system information tool written in pure C"
arch=('x86_64')
url=""
license=('MIT')
depends=('glibc')
source=('cc.c' 'Makefile')
sha256sums=('SKIP'
            'SKIP')

build() {
    cd "$srcdir"
    make
}

package() {
    cd "$srcdir"
    make DESTDIR="$pkgdir" install
}
