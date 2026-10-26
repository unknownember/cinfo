# Maintainer: kori2 <nothankyou@gmail.com>
pkgname=cinfo
pkgver=1.1.3
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
sha256sums=('91c90eba8d23f941c943c4e75217335ef55cc6cf8faca25aeacd57039a34a0b3' 'adbc6503191c1bb5b82fb85a36ebf694ff4fde275cccfce8d727efe8310aa54b' '00742bca9f1a5332efc41b93a272950897bc845dca30612a6f05d88aab127c20')

build() {
    cd "$srcdir"
    make
}

package() {
    cd "$srcdir"
    install -Dm755 cinfo "$pkgdir/usr/bin/cinfo"
    install -Dm644 LICENSE "$pkgdir/usr/share/licenses/$pkgname/LICENSE"
}
