// ------------------------------------------------------------------------------
// Primitive Updater for apps released on Github - Made for Greyhound - by Scobalula
// Copyright(c) 2018 Scobalula/Philip
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// ------------------------------------------------------------------------------
using System;
using System.Diagnostics;
using System.IO;
using System.IO.Compression;
using System.Windows;
using System.Threading;
using System.ComponentModel;
using System.Net;
using System.Reflection;
using Octokit;
using System.Globalization;
using System.Security.Policy;

namespace GreyhoundUpdater
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        /// <summary>
        /// Latest Github Release Asset
        /// </summary>
        ReleaseAsset LatestReleaseAsset { get; set; }

        /// <summary>
        /// Gets Application Directory
        /// </summary>
        public string ApplicationDirectory { get; set; }

        /// <summary>
        /// Name of the Github Repo to Query
        /// </summary>
        public string GithubRepoName { get; set; }

        /// <summary>
        /// Name of the User Account that owns the Repo
        /// </summary>
        public string GithubUserName { get; set; }

        /// <summary>
        /// Name of the User Account that owns the Repo
        /// </summary>
        public string PackageIndexRepoName { get; set; }

        /// <summary>
        /// Name of the Application we're updating
        /// </summary>
        public string ApplicationName { get; set; }

        /// <summary>
        /// Name of the assembly of the Application we're updating
        /// </summary>
        public string AssemblyName { get; set; }

        /// <summary>
        /// Whether we want to download via the Client or Open a browser
        /// </summary>
        public bool DownloadViaClient = true;

        /// <summary>
        /// A tracker variable to check if we're updating
        /// </summary>
        public bool Updating = false;

        /// <summary>
        /// A tracker variable to check if we're updating
        /// </summary>
        public bool PackageIndexMode = false;

        /// <summary>
        /// Gets or Sets the Client
        /// </summary>
        GitHubClient Client { get; set; }

        /// <summary>
        /// Main Entry Point for WPF
        /// </summary>
        public MainWindow()
        {
            // Check command line arguments
            if (!GetCommandLineArguments())
            {
                MessageBox.Show("This application is intended to be opened by its parent program.", "Updater", MessageBoxButton.OK, MessageBoxImage.Exclamation);
                Close();
            }
            // TODO: Really improve the hacky slapped in package index logic, maybe merge everything into 1 clean system
            // var result = MessageBox.Show($"{ApplicationName} can check for updates for both itself and new items from its index repo.\n\nWould you like to enable automatic updates?", "Updater", MessageBoxButton.YesNo, MessageBoxImage.Information);
            // Try grab and open the window, if we fail, exit
            try
            {
                // Query Github Releases
                Client = new GitHubClient(new ProductHeaderValue(ApplicationName));

                // Debug purposes, need higher rate limit
                if(File.Exists(@"C:\Visual Studio\Auth\OctoKitAuth.txt"))
                    Client.Credentials = new Credentials(File.ReadAllText(@"C:\Visual Studio\Auth\OctoKitAuth.txt"));

                try
                {
                    if (PackageIndexRequiresUpdate(Client))
                    {
                        InitializeComponent();
                        // Set Version
                        LatestVersion.Content = String.Format("Package Index Update");
                        // Set Titles
                        Title = ApplicationName + " Update";
                        TitleLabel.Content = ApplicationName + " Update";
                        // Append change log
                        Changes.AppendText("An update is available for the Package Index. This includes new hashes for content.");
                        // Set index mode
                        PackageIndexMode = true;
                    }
                }
                catch { }
                // We're updating
                if(!PackageIndexMode)
                {
                    var releases = Client.Repository.Release.GetAll(GithubUserName, GithubRepoName);
                    var latestRelease = releases.Result[0];
                    // Query File Version
                    var fileInfo = FileVersionInfo.GetVersionInfo(AssemblyName);
                    // Convert to Version Objects for comparison
                    var currentVersion = new Version(fileInfo.FileVersion);
                    var remoteVersion = new Version(latestRelease.TagName);
                    // Set Latest Release Asset
                    LatestReleaseAsset = latestRelease.Assets[0];
                    // Check current Version against Remote
                    if (currentVersion < remoteVersion)
                    {
                        InitializeComponent();
                        // Set Version
                        LatestVersion.Content = String.Format("Update {0}", remoteVersion);
                        // Set Titles
                        Title = ApplicationName + " Update";
                        TitleLabel.Content = ApplicationName + " Update";
                        // Append change log
                        Changes.AppendText(latestRelease.Body);
                    }
                    else
                    {
                        Close();
                    }
                }
            }
            catch
            {
               Close();
            }
        }

        /// <summary>
        /// Updates the package index
        /// </summary>
        private void UpdatePackageIndexFolder(GitHubClient client)
        {
            try
            {
                // Download entire repo
                var archiveBytes = client.Repository.Content.GetArchive(GithubUserName, PackageIndexRepoName, ArchiveFormat.Zipball);

                using (ZipArchive archive = new ZipArchive(new MemoryStream(archiveBytes.Result)))
                {
                    foreach (var entry in archive.Entries)
                    {
                        if(Path.GetExtension(entry.FullName) == ".csv")
                        {
                            CompileFromCSV(entry.Open(), Path.GetFileNameWithoutExtension(entry.FullName));
                        }
                    }
                }
            }
            catch
            {

            }
            finally
            {
                // Try launch Greyhound again
                try
                {
                    Process.Start(Path.Combine(ApplicationDirectory, AssemblyName));
                }
                catch
                {
                    Updating = false;
                    MessageBox.Show($"Failed to execute {AssemblyName}.", "ERROR", MessageBoxButton.OK, MessageBoxImage.Error);
                    Close();
                }
                // Invoke UI Changes and launch Finalize method
                Dispatcher.BeginInvoke(new Action(() =>
                {
                    ProgressInfo.Content = "Finalizing....";
                    Progress.IsIndeterminate = true;
                }));
                // Set back
                Updating = false;
                // Delay the exit
                DelayExit();
            }
        }

        /// <summary>
        /// Checks if we should update the package index but comparing the commit hash
        /// </summary>
        private bool PackageIndexRequiresUpdate(GitHubClient client)
        {
            if (string.IsNullOrWhiteSpace(PackageIndexRepoName))
                return false;

            // We're basically using the latest SHA to check, if it's different, something has
            // changed and therefore we need to refresh the package index repo
            var commits = client.Repository.Commit.GetAll(GithubUserName, PackageIndexRepoName, new ApiOptions() { PageCount = 1, PageSize = 1 });
            var topCommit = commits.Result[0];

            var cache = Path.Combine(ApplicationDirectory, "package_index", "commit_cache.dat");

            if (!File.Exists(cache))
            {
                Directory.CreateDirectory(Path.GetDirectoryName(cache));
                File.WriteAllText(cache, topCommit.Sha);
                return true;
            }

            var commitCache = File.ReadAllText(cache);

            // Debug for people doing hash entries, do not want updates
            if(commitCache == "-1")
            {
                return false;
            }

            // Our commits are different, we must refresh the package index
            if(commitCache != topCommit.Sha)
            {
                File.WriteAllText(cache, topCommit.Sha);
                return true;
            }

            return false;
        }

        /// <summary>
        /// Compiles a WNI file from a CSV file
        /// </summary>
        private void CompileFromCSV(Stream stream, string name)
        {
            var dir = Path.Combine(ApplicationDirectory, "package_index");
            Directory.CreateDirectory(dir);
            var wniFile = Path.Combine(dir, $"{name}.wni");
            var index = new PackageIndex();

            using(var reader = new StreamReader(stream))
            {
                string line = null;

                while ((line = reader.ReadLine()) != null)
                {
                    string lineTrim = line.Trim();

                    if (!lineTrim.StartsWith("#"))
                    {
                        string[] lineSplit = lineTrim.Split(',');

                        if (lineSplit.Length > 1)
                        {
                            if (ulong.TryParse(lineSplit[0], NumberStyles.HexNumber, CultureInfo.CurrentCulture, out ulong id))
                            {
                                id &= 0xFFFFFFFFFFFFFFF;

                                if (index.Entries.ContainsKey(id))
                                    continue;

                                index.Entries[id] = lineSplit[1];
                            }
                        }
                    }
                }
            }

            // Save
            index.Save(wniFile);
        }

        /// <summary>
        /// Gets Update Info from Command Line Arguments
        /// </summary>
        /// <returns></returns>
        private bool GetCommandLineArguments()
        {
            // Get arguments
            string[] args = Environment.GetCommandLineArgs();
            // Check did we receive the correct number of arguments (we want 5, so we check 5 since we're including this assembly)
            if (args.Length < 7)
                return false;
            // Assign data
            GithubUserName       = args[1];
            GithubRepoName       = args[2];
            ApplicationName      = args[3];
            AssemblyName         = args[4];
            ApplicationDirectory = args[5];
            DownloadViaClient    = args[6] == "true";
            if (args.Length >= 8)
                PackageIndexRepoName = args[6];
            else
                PackageIndexRepoName = "GreyhoundPackageIndex";
            // Got it
            return true;
        }

        /// <summary>
        /// Cancel Button Logic (Closes Window)
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void Cancel_Click(object sender, RoutedEventArgs e)
        {
            Close();
        }

        /// <summary>
        /// Download Button Logic
        /// </summary>
        private void DownloadNow_Click(object sender, RoutedEventArgs e)
        {
            // Check whether we want to download now, or open a browser
            if (DownloadViaClient)
            {
                // Change to Updating
                Updating = true;
                // Change Views
                ChangelogGrid.Visibility = Visibility.Hidden;
                DownloadNow.Visibility = Visibility.Hidden;
                Cancel.Visibility = Visibility.Hidden;
                ProgressGrid.Visibility = Visibility.Visible;
                // Get Processes matching Greyhound
                Process[] processes = Process.GetProcessesByName(AssemblyName.Split('.')[0]);
                // Loop and Kill any instances of Greyhound
                foreach (var process in processes)
                    process.Kill();
                // Check mode
                if (PackageIndexMode)
                {
                    ProgressInfo.Content = "Updating Package Index....";
                    Progress.IsIndeterminate = true;
                    // Create and set download methods and data
                    new Thread(() => {
                        UpdatePackageIndexFolder(Client);
                    }).Start();
                }
                else
                {
                    // Initiate the Download
                    InitiateDownload();
                }
            }
            else
            {
                Process.Start(LatestReleaseAsset.BrowserDownloadUrl);
                Close();
            }
        }

        /// <summary>
        /// Initiates Github Index Download
        /// </summary>
        private void InitiateIndexDownload()
        {
            // Create and set download methods and data
            new Thread(() => {
                WebClient client = new WebClient();
                client.DownloadProgressChanged += new DownloadProgressChangedEventHandler(UpdateProgress);
                client.DownloadFileCompleted += new AsyncCompletedEventHandler(DownloadComplete);
                client.DownloadFileAsync(new Uri(LatestReleaseAsset.BrowserDownloadUrl), Path.Combine(ApplicationDirectory, "Update.zip"));
            }).Start();
        }

        /// <summary>
        /// Initiates Github Asset Download
        /// </summary>
        private void InitiateDownload()
        {
            // Create and set download methods and data
            new Thread(() => {
                WebClient client = new WebClient();
                client.DownloadProgressChanged += new DownloadProgressChangedEventHandler(UpdateProgress);
                client.DownloadFileCompleted += new AsyncCompletedEventHandler(DownloadComplete);
                client.DownloadFileAsync(new Uri(LatestReleaseAsset.BrowserDownloadUrl), Path.Combine(ApplicationDirectory, "Update.zip"));
            }).Start();
        }

        /// <summary>
        /// Updates Progress Bar during Download
        /// </summary>
        private void UpdateProgress(object sender, DownloadProgressChangedEventArgs e)
        {
            Dispatcher.BeginInvoke(new Action(() => 
            {
                Progress.Value = int.Parse(Math.Truncate(double.Parse(e.BytesReceived.ToString()) / double.Parse(e.TotalBytesToReceive.ToString()) * 100).ToString());
            }));
        }

        /// <summary>
        /// Extracts and relaunches Greyhound on Download complete
        /// </summary>
        private void DownloadComplete(object sender, AsyncCompletedEventArgs e)
        {
            // Invoke UI Changes and launch Finalize method
            Dispatcher.BeginInvoke(new Action(() =>
            {
                ProgressInfo.Content = "Finalizing....";
                Progress.IsIndeterminate = true;
                FinalizeUpdate();
            }));
        }

        /// <summary>
        /// Finalizes the Update by Unpacking the assets and relaunching Greyhound
        /// </summary>
        private void FinalizeUpdate()
        {
            // Check if Update Zip was actually downloaded
            if(File.Exists(Path.Combine(ApplicationDirectory, Path.Combine(ApplicationDirectory, "Update.zip"))))
            {
                // Try unpack, we should not fail unless permission issues, etc. occur.
                try
                {
                    using (ZipArchive archive = new ZipArchive(new FileStream(Path.Combine(ApplicationDirectory, "Update.zip"), System.IO.FileMode.Open)))
                    {
                        foreach (var entry in archive.Entries)
                        {
                            if (!String.IsNullOrWhiteSpace(entry.Name))
                            {
                                string resultingFile = Path.Combine(ApplicationDirectory, entry.FullName);
                                Directory.CreateDirectory(Path.GetDirectoryName(resultingFile));
                                entry.ExtractToFile(resultingFile, true);
                            }
                        }
                    }
                }
                catch
                {
                    Updating = false;
                    MessageBox.Show("Update failed. Could not unpack Update.zip, potentially corrupt download.", "ERROR", MessageBoxButton.OK, MessageBoxImage.Error);
                    Close();
                }
            }
            else
            {
                Updating = false;
                MessageBox.Show("Update failed. Could not find Update.zip.", "ERROR", MessageBoxButton.OK, MessageBoxImage.Error);
                Close();
            }
            // Try launch Greyhound again
            try
            {
                Process.Start(Path.Combine(ApplicationDirectory, AssemblyName));
            }
            catch
            {
                Updating = false;
                MessageBox.Show($"Failed to execute {AssemblyName}.", "ERROR", MessageBoxButton.OK, MessageBoxImage.Error);
                Close();
            }
            // Set back
            Updating = false;
            // Delete Update File
            if(File.Exists(Path.Combine(ApplicationDirectory, "Update.zip")))
            {
                try
                {
                    File.Delete(Path.Combine(ApplicationDirectory, "Update.zip"));
                }
                catch
                {
                    // Mission failed, we'll get 'em next time
                    MessageBox.Show("Failed to delete Update.zip, please delete it manually", "ERROR", MessageBoxButton.OK, MessageBoxImage.Exclamation);
                }
            }
            // Delay the exit
            DelayExit();
        }

        /// <summary>
        /// Delays exit to give time for Greyhound to start
        /// </summary>
        private void DelayExit()
        {
            new Thread(() => 
            {
                Thread.Sleep(1000);
                Dispatcher.BeginInvoke(new Action(() =>
                {
                    Close();
                }));
            }).Start();
        }

        /// <summary>
        /// Checks if we're updating on close
        /// </summary>
        private void Window_Closing(object sender, CancelEventArgs e)
        {
            if (Updating)
                e.Cancel = true;
        }
    }
}
