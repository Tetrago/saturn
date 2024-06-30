{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-24.05";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { nixpkgs, flake-utils, ... }: flake-utils.lib.eachSystem [
    "x86_64-linux"
  ] (system: let pkgs = nixpkgs.legacyPackages.${system}; in {
    devShells.default = (pkgs.mkShell.override { stdenv = pkgs.llvmPackages.stdenv; }) {
      buildInputs = with pkgs; [
        glfw
        vulkan-headers
        vulkan-loader
      ];

      packages = with pkgs; [
        cmake
        gdb
        ninja
      ];

      VK_LAYER_PATH = "${pkgs.vulkan-validation-layers}/share/vulkan/explicit_layer.d";
    };
  });
}
