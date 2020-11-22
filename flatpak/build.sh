#!/usr/bin/env bash
flatpak install flathub org.gnome.Platform//3.38 org.gnome.Sdk//3.38
flatpak-builder --force-clean --arch=x86_64 build-dir com.giuspen.cherrytree.json
flatpak-builder --run build-dir com.giuspen.cherrytree.json cherrytree
#flatpak-builder --force-clean --arch=x86_64 --repo=repo build-dir com.giuspen.cherrytree.json
#flatpak build-bundle --arch=x86_64 repo cherrytree-0.39.3.x86_64.flatpak com.giuspen.cherrytree
#flatpak install cherrytree-0.39.3.x86_64.flatpak
