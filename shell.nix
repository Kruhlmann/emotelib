{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  buildInputs = [
    pkgs.gcc
    pkgs.curl
    pkgs.pkg-config
    pkgs.openssl
  ];
}
