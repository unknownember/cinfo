# Maintainer: kori2 <nothankyou@gmail.com>
pkgname=cinfo
pkgver=1.1.0
pkgrel=1
pkgdesc="A tiny, fast Linux system information tool written in pure C"
arch=('x86_64')
url="https://github.com/unknownember/cinfo"
license=('MIT')
depends=('glibc')
makedepends=('make')
source=("$pkgname-$pkgver.tar.gz::https://github.com/unknownember/cinfo/archive/refs/tags/v$pkgver.tar.gz")
sha256sums=('c5e815aa5fbbf3eb8e28f0be9d3c899d0171d0737997a59f58a04be314f638d1')

build() {
    cd "$srcdir/$pkgname-$pkgver"
    make
}

package() {
    cd "$srcdir/$pkgname-$pkgver"
    install -Dm755 cinfo "$pkgdir/usr/bin/cinfo"
    install -Dm644 LICENSE "$pkgdir/usr/share/licenses/$pkgname/LICENSE"
}
