# 19 - Sysfs

Initially, `procfs` was intended for querying information about the main kernel modules. However, with the explosion of device drivers that appeared with the years, along with more creative ways of using them (also via write operations), `procfs` became a bit chaotic.

It was decided to create `sysfs` for new device drivers, trying to leave `procfs` for just reading information from the most "core" modules.

This exercise is very similar to **18 - Procfs**. The difference relies on the way the directory and files are created. In the `sysfs` implementation approach, the directories are considered `kobjects`, and they have a specific `kobj_attribute` structure that contains their attributes and their R/W callbacks. In this matter, the read operations are known as "**show**" operations, where "**store**" word is related to the "write" operations.

**NOTE**: due to recent kernel changes, the code in this version of the exercise will not match exactly the version in the Youtube tutorial.
