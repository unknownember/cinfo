# Maintainer: kori2 <nothankyou@gmail.com>
pkgname=cinfo
pkgver=1.0.1
pkgrel=1
pkgdesc="A tiny, fast Linux system information tool written in pure C"
arch=('x86_64')
url="https://github.com/unknownember/cinfo"
license=('MIT')
depends=('glibc')
makedepends=('make')
source=("$pkgname-$pkgver.tar.gz::https://github.com/unknownember/cinfo/archive/refs/tags/v$pkgver.tar.gz")
sha256sums=('f81fd09a56ae3828daf954061c03dc00b0c69f62eebab266f943b3eaf53c0363')

build() {
    cd "$srcdir/$pkgname-$pkgver"
    make
}

package() {
    cd "$srcdir/$pkgname-$pkgver"
    install -Dm755 cinfo "$pkgdir/usr/bin/cinfo"
}
