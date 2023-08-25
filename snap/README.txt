
- This was largely copied from Inkscape's https://gitlab.com/inkscape/inkscape/-/blob/master/snap/snapcraft.yaml

- Cherrytree's snap repo is at https://snapcraft.io/cherrytree



- Build (https://snapcraft.io/docs/snapcraft-overview):

sudo snap install snapcraft --classic

sudo usermod -aG lxd giuspen
newgrp lxd

export SNAPCRAFT_BUILD_ENVIRONMENT_CPU=8
snapcraft



- Install

sudo snap install cherrytree_1.0.1_amd64.snap --dangerous --devmode



- Upload

snapcraft login
snapcraft upload --release=stable cherrytree_1.0.1_amd64.snap
snapcraft logout

NOTE: if it fails login with craft-store error: Credentials found for 'snapcraft' on 'dashboard.snapcraft.io'
      run 'seahorse' and remove the login to 'dashboard.snapcraft.io', likely last time you missed the logout
