# Maintainer: kori2 <nothankyou@gmail.com>
pkgname=cinfo
pkgver=1.1.1
pkgrel=1
pkgdesc="A tiny, fast Linux system information tool written in pure C"
arch=('x86_64')
url="https://github.com/unknownember/cinfo"
license=('MIT')
depends=('glibc')
makedepends=('make')
source=("$pkgname-$pkgver.tar.gz::https://github.com/unknownember/cinfo/archive/refs/tags/v$pkgver.tar.gz")
sha256sums=('2d2784b2fcad3138b8cc2047d7c5f637701822c539bef284a10cfd557990db82')

build() {
    cd "$srcdir/$pkgname-$pkgver"
    make
}

package() {
    cd "$srcdir/$pkgname-$pkgver"
    install -Dm755 cinfo "$pkgdir/usr/bin/cinfo"
    install -Dm644 LICENSE "$pkgdir/usr/share/licenses/$pkgname/LICENSE"
}
