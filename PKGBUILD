# Maintainer: kori2 <nothankyou@gmail.com>
pkgname=cinfo
pkgver=1.1.5
pkgrel=1
pkgdesc="A tiny, fast Linux system information tool written in pure C"
arch=('x86_64')
url="https://github.com/unknownember/cinfo"
license=('MIT')
depends=('glibc')
makedepends=('make')
source=(
    "Makefile::https://raw.githubusercontent.com/unknownember/cinfo/v$pkgver/Makefile"
    "cc.c::https://raw.githubusercontent.com/unknownember/cinfo/v$pkgver/cc.c"
    "LICENSE::https://raw.githubusercontent.com/unknownember/cinfo/v$pkgver/LICENSE"
)
sha256sums=('9aa0ecc32745a84e12adb61dfdf37bfaa832ca6a97c3244f503028cb49f36d9d'
            'ca5b35bc5e0b5ef3b5433e42154dbd0dd6b240c8e8bc007b0f5ca70641f20592'
            '00742bca9f1a5332efc41b93a272950897bc845dca30612a6f05d88aab127c20')

build() {
    cd "$srcdir"
    make
}

package() {
    cd "$srcdir"
    install -Dm755 cinfo "$pkgdir/usr/bin/cinfo"
    install -Dm644 LICENSE "$pkgdir/usr/share/licenses/$pkgname/LICENSE"
}
