# tdnf plugins

plugins are a way to extend tdnf functionality.

tdnf plugins follow the same config and command line conventions as yum.

## enable plugins
tdnf will install with plugins deactivated by default. This is because the primary switch
to turn on plugins is in tdnf conf file (/etc/tdnf/tdnf.conf by default).
To enable plugins, the config file should have

```plugins=1```

## plugin discovery
tdnf will look in ```/etc/tdnf/pluginconf.d``` by default for plugin configurations.
For all config files with ```enabled=1``` set, tdnf will look for a corresponding
shared library in ```<libdir>/tdnf-plugins/<plugin>/lib<plugin>.so```.
```pluginpath``` and ```pluginconfpath``` are config settings to change default paths.

## overriding plugin load
tdnf allows command line overrides with ```--enableplugin=<plugin>``` to enable a plugin
that is deactivated in the corresponding plugin config file.
Similarly, ```--disableplugin=<plugin>``` can be used to deactivate a plugin which is
otherwise enabled in it's corresponding config file.

For eg: ```tdnf --disableplugin=* --enableplugin=myplugin``` will deactivate all plugins
but ```myplugin``` that is subsequently enabled. The deactivate and enable overrides are
sequential, cumulative and support globs.
Therefore, it does matter where you place the deactivate option.
