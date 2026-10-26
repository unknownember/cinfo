# Maintainer: kori2 <nothankyou@gmail.com>
pkgname=cinfo
pkgver=1.1.2
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
sha256sums=('b228ea1e950d72e57e49adb91767a92e19757190edacd408e8d8285c0123c01f' '59e709c13f2805ec7cc9ffcee0c41183c9630a633fc82eb3e5d3a5ba59534259' '00742bca9f1a5332efc41b93a272950897bc845dca30612a6f05d88aab127c20')

build() {
    cd "$srcdir"
    make
}

package() {
    cd "$srcdir"
    install -Dm755 cinfo "$pkgdir/usr/bin/cinfo"
    install -Dm644 LICENSE "$pkgdir/usr/share/licenses/$pkgname/LICENSE"
}
