# Maintainer: kori2 <nothankyou@gmail.com>
pkgname=cinfo
pkgver=1.1.4
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
sha256sums=('bd94bd98edeffba608eb15b254c19303f6077eac6f9241cb60b17b54e3d8ba19'
            '7c158926d8e2ea5d39929e08444df62fde4b282a380cd5734b68aa2572134b73'
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
