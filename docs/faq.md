---
layout: default
title: Frequently Asked Questions
nav_order: 4
---

# Frequently Asked Questions
{: .no_toc }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

This is a list of questions frequently asked in our Discord. We will continue to populate this with questions that commonly pop up.

If you're questions isn't listed here or you want to report an issue please join our Discord:

[Join Our Discord for More Support](https://discord.gg/RyqyThu){: .btn .fs-5 .mb-4 .mb-md-0 }

## Greyhound says it cannot find a game despite it being open?

The first thing to do is to ensure you are running a valid copy of the game from the official distributors. Cracked and modified copies are not officially supported and tend not to work due to being outdated or containing edits that cause issues with Greyhound.

If you have checked that off, you can try running Greyhound as admin, this is a common fix for many people.

## Greyhound is not exporting some images with models, they are even listed in the .txt file?

This can happen due to various reasons. A common issue is long file paths that go over a limit (usually between 250-260 characters in total) that are not supported by a lot of applications. In this case moving the folder to reduce the total length of the file path can help. I personally tend to keep Greyhound in a folder on my external drive with the path: `G:\Tools\GH`

Another common problem is the images might be loaded images which are handled in such a way in some titles that they cannot be exported by Greyhound. This is usually less of an issue as it's commonly done for UI images or single color images that are small in size.

## Greyhound is not finding the asset I am looking for?

Greyhound lists every asset it finds, if you cannot find the asset you want, you're probably not looking for the right name. A prime example of this are character and weapon models, where actor and code names are heavily used such as `mpapa7` for Call of Duty: Modern Warfare's MP7 and `bailey` for Call of Duty: Vanguard's Polina (actor name).

This can be tricky to work around, and if you're really stuck, you can join our Discord above. Usually someone has tried to find the exact asset you're looking for and will help you out.

## Can I get banned for using Greyhound?

Greyhound itself does not interfere with the game and simply performs Read operations on the game and its files, it also is primarily for people using it for ethical purposes such as non-commercial mods in older CoD Mod Tools Packages and 3D Artists for YouTube thumbnails and I've yet to hear of the developers targeting it.

That being said, using Greyhound is completely at your own risk and you are responsible for anything that happens while using it.

## I want to get into making 3D Renders using Greyhound, how can I do that?

There are tons of resources, tutorials, and software available to help you get into making 3D art using Greyhound.

You can join our Discord above and get help from various community members on the matter alongside what's available online.

## I want to use Greyhound for datamining and, how can I do that?

No.

## I want to compile Greyhound, how can I do that?

While I cannot give support for working with the source code, it is very easy to build. An install of Visual Studio 2019 or greater with MFC should compile Greyhound out of the box, I've done it several times with a fresh install of Windows.