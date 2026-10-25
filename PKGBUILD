# Maintainer: kori2 <nothankyou@gmail.com>
pkgname=cinfo
pkgver=1.0.2
pkgrel=1
pkgdesc="A tiny, fast Linux system information tool written in pure C"
arch=('x86_64')
url="https://github.com/unknownember/cinfo"
license=('MIT')
depends=('glibc')
makedepends=('make')
source=("$pkgname-$pkgver.tar.gz::https://github.com/unknownember/cinfo/archive/refs/tags/v$pkgver.tar.gz")
sha256sums=('7c197a858dfe3b484001a6ccf1e3465abf1668d627be6a07e4e128d7447785e7')

build() {
    cd "$srcdir/$pkgname-$pkgver"
    make
}

package() {
    cd "$srcdir/$pkgname-$pkgver"
    install -Dm755 cinfo "$pkgdir/usr/bin/cinfo"
    install -Dm644 LICENSE "$pkgdir/usr/share/licenses/$pkgname/LICENSE"
}
