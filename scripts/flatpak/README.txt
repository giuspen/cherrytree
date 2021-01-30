
- This was started after Inkscape's https://github.com/flathub/org.inkscape.Inkscape

- Cherrytree's flathub repo is at https://github.com/flathub/com.giuspen.cherrytree/

- This is not meant to stay at pace with the official com.giuspen.cherrytree.json but will be upgraded only when changes other than url/sha256 are issued



- Build and run Flatpak locally:

sudo apt install flatpak flatpak-builder gnome-software-plugin-flatpak

flatpak remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo

flatpak install flathub org.gnome.Platform//3.38 org.gnome.Sdk//3.38

flatpak-builder --force-clean --arch=x86_64 build-dir com.giuspen.cherrytree.json

flatpak-builder --run build-dir com.giuspen.cherrytree.json cherrytree



- Build redistributable flatpak and install it

flatpak-builder --force-clean --arch=x86_64 --repo=repo build-dir com.giuspen.cherrytree.json

flatpak build-bundle --arch=x86_64 repo cherrytree-0.39.3.x86_64.flatpak com.giuspen.cherrytree

flatpak install cherrytree-0.39.3.x86_64.flatpak
