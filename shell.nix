{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  buildInputs = [
    pkgs.gcc
    pkgs.curl
    pkgs.gd
    pkgs.pkg-config
    pkgs.openssl
    pkgs.ffmpeg
  ];
}
