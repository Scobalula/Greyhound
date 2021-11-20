---
layout: default
title: Search
nav_order: 3
---

# Search
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

Greyhound allows you to find what you need based off various asset properties. Search queries are seperated using commas which allows you to enter multiple queries into the box at once.

# Basic Search

The simplest property you can search for is the asset name, you can do this by simply entering comma seperated text into the search box with text you think might be in the name of the asset you are search for. For example if we had an asset with the following name:

```
attachment_wm_stock_heavy_mpapa7_v5
```

We can search for:

```
mpapa7
```

And it will show up in the search results. However if we instead wanted to list all character models and heads, we could search for:


```
body,head
``` 

And it would show all assets that contain body and head. If we instead wanted to exclude body and head, we can simply add an exclamation mark `!`, like so:

```
!body,head
```

# Advanced Search

Greyhound also allows you to search for assets based off various other properties depending on the asset type. This can be useful for finding groups of assets with known characteristics such as large bone counts.

To search for one of the properties below simply type the property name along with the value, for example:

```
lodcount:1
```

You can also search for multiple values or properties, for example:

```
lodcount:1,lodcount:2,bonecount:5
```

Some properties support range based searching, for example:

```
bonecount:>100,bonecount:<200
```

Make sure to check the labels for information such as the asset types the property supports and what games the property supports. Some games do not support all properties. If an asset type is not supported by the properties you are searching for, they will still be listed in your search.

## LOD Count

<div class="code-example" markdown="1">
Supported by:

Models
{: .label .label-blue }
All Games
{: .label .label-green }

The `lodcount` property allows you to search based off the LOD (Level of Detail) count of an asset.

Examples:
</div>
```
lodcount:1

lodcount:>1

lodcount:<4,lodcount:>5
```

## Bone Count

<div class="code-example" markdown="1">
Supported by:

Models
{: .label .label-blue }
Animations
{: .label .label-blue }
All Games
{: .label .label-green }

The `bonecount` property allows you to search based off the bone count of an asset.

Examples:
</div>
```
bonecount:150

bonecount:>15

bonecount:<10,bonecount:>50
```

## Frame Count

<div class="code-example" markdown="1">
Supported by:

Animations
{: .label .label-blue }
All Games
{: .label .label-green }

The `framecount` property allows you to search based off the frame count of an asset.

Examples:
</div>
```
framecount:150

framecount:>15

framecount:<10,framecount:>50
```

## Framerate

<div class="code-example" markdown="1">
Supported by:

Animations
{: .label .label-blue }
All Games
{: .label .label-green }

The `framerate` property allows you to search based off the framerate of an asset.

Examples:
</div>
```
framerate:60

framerate:>15

framerate:>30,framerate:<60
```

## Width

<div class="code-example" markdown="1">
Supported by:

Images
{: .label .label-blue }
All Games
{: .label .label-green }

The `width` property allows you to search based off the width of an asset in pixels.

Examples:
</div>
```
width:2048

width:>1024

width:>1024,width:<4096
```

## Height

<div class="code-example" markdown="1">
Supported by:

Images
{: .label .label-blue }
All Games
{: .label .label-green }

The `height` property allows you to search based off the height of an asset in pixels.

Examples:
</div>
```
height:2048

height:>1024

height:>1024,height:<4096
```

## Length

<div class="code-example" markdown="1">
Supported by:

Sounds
{: .label .label-blue }
All Games
{: .label .label-green }

The `length` property allows you to search based off the length of an asset in milliseconds.

Examples:
</div>
```
Length:100000

Length:>10000

Length:>10000,Length:<100000
```

## Bone Name

<div class="code-example" markdown="1">
Supported by:

Models
{: .label .label-blue }
Animations
{: .label .label-blue }
Call of Duty: Black Ops Cold War
{: .label .label-green }
Call of Duty: Black Ops IIII
{: .label .label-green }

The `bonename` property allows you to search based off the bone names of an asset. This checks the entire list of bones for the asset.

Examples:
</div>
```
bonename:tag_torso

Length:tag_torso,Length:tag_origin
```