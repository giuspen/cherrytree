
- This was started after Inkscape's https://github.com/flathub/org.inkscape.Inkscape

- Cherrytree's flathub repo is at https://github.com/flathub/com.giuspen.cherrytree

- This is not meant to stay at pace with the official com.giuspen.cherrytree.json but will be upgraded only when changes other than url/sha256 are issued



- Build and run Flatpak locally:

sudo apt install flatpak flatpak-builder gnome-software-plugin-flatpak

flatpak remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo

flatpak install flathub org.gnome.Platform//42 org.gnome.Sdk//42

flatpak-builder --force-clean --arch=x86_64 build-dir com.giuspen.cherrytree.json

# NOTE: access to the file system will not work unless you also install it once (see below)
flatpak-builder --run build-dir com.giuspen.cherrytree.json cherrytree



- Build redistributable flatpak and install it

flatpak-builder --force-clean --arch=x86_64 --repo=repo build-dir com.giuspen.cherrytree.json

flatpak build-bundle --arch=x86_64 repo cherrytree-0.99.47.x86_64.flatpak com.giuspen.cherrytree

flatpak install cherrytree-0.99.47.x86_64.flatpak
