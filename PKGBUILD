# Maintainer: kori2 <nothankyou@gmail.com>
pkgname=cinfo
pkgver=1.1.6
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
sha256sums=('21301dc0f5976e540f873033ce5289ef6ea8a5cc904288aee8df9097349a9a16'
            'a9cd7833bff2966664c5da875077289ef3f14ad663b68365afeab4c915ddabc9'
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
