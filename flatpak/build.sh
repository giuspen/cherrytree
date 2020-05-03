#!/usr/bin/env bash
flatpak install flathub org.freedesktop.Platform//19.08 org.freedesktop.Sdk//19.08
flatpak-builder --force-clean --arch=x86_64 build-dir com.giuspen.cherrytree.json
flatpak-builder --run build-dir com.giuspen.cherrytree.json cherrytree
#flatpak-builder --force-clean --arch=x86_64 --repo=repo build-dir com.giuspen.cherrytree.json
#flatpak build-bundle --arch=x86_64 repo cherrytree-0.39.3.x86_64.flatpak com.giuspen.cherrytree
#flatpak install cherrytree-0.39.3.x86_64.flatpak
