
- This was largely copied from Inkscape's https://gitlab.com/inkscape/inkscape/-/blob/master/snap/snapcraft.yaml

- Cherrytree's snap repo is at https://snapcraft.io/cherrytree



- Build (https://snapcraft.io/docs/snapcraft-overview):

sudo snap install snapcraft --classic

sudo usermod -aG lxd giuspen
newgrp lxd

export SNAPCRAFT_BUILD_ENVIRONMENT_CPU=8
snapcraft



- Install

sudo snap install cherrytree_0.99.48_amd64.snap --dangerous --devmode
