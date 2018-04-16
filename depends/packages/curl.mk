package=curl
$(package)_version=7.43.0
$(package)_download_path=https://curl.haxx.se/download
$(package)_file_name=$(package)-$($(package)_version).tar.gz
$(package)_dependencies=openssl
$(package)_sha256_hash=1a084da1edbfc3bd632861358b26af45ba91aaadfb15d6482de55748b8dfc693

define $(package)_set_vars
$(package)_config_opts += --with-ssl=$(host_prefix)
$(package)_config_opts += --disable-shared
$(package)_config_opts += --enable-hidden-symbols
$(package)_config_opts += --enable-threaded-resolver
$(package)_config_opts += --without-librtmp
$(package)_config_opts += --without-winidn
$(package)_config_opts += --without-libidn
$(package)_config_opts += --without-zlib
$(package)_config_opts += --disable-ldap
$(package)_config_opts += --disable-ldaps
$(package)_config_opts += --disable-rtsp
$(package)_config_opts += --disable-dict
$(package)_config_opts += --disable-telnet
$(package)_config_opts += --disable-tftp
$(package)_config_opts += --disable-pop3
$(package)_config_opts += --disable-imap
$(package)_config_opts += --disable-smb
$(package)_config_opts += --disable-smtp
$(package)_config_opts += --disable-gopher
$(package)_config_opts += --disable-manual
$(package)_config_opts_linux = --with-pic
$(package)_config_opts_darwin = --with-darwinssl
$(package)_config_opts_mingw32 = --with-winssl
endef

define $(package)_preprocess_cmds
  sed -i.old "s|-lgdi32 \$$$$LIBS|\$$$$LIBS -lgdi32|" configure
endef

define $(package)_config_cmds
  $($(package)_autoconf)
endef

define $(package)_build_cmds
  $(MAKE)
endef

define $(package)_stage_cmds
  $(MAKE) DESTDIR=$($(package)_staging_dir) install
endef
