FROM fedora:rawhide

RUN dnf install -y dnf-plugins-core uncrustify findutils git \
    && dnf builddep -y epiphany \
    && dnf clean all
