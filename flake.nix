{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { nixpkgs, flake-utils, ... }: flake-utils.lib.eachSystem [
    "x86_64-linux"
  ] (system: let pkgs = nixpkgs.legacyPackages.${system}; in {
    devShells.default = (pkgs.mkShell.override { stdenv = pkgs.llvmPackages.stdenv; }) {
      buildInputs = with pkgs; [
        glfw
        shaderc
        vulkan-headers
        vulkan-loader
        vulkan-memory-allocator
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
