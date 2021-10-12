using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using LZ4;

namespace GreyhoundUpdater
{
    /// <summary>
    /// A class to handle Greyhound/Wraith Package Index Files
    /// </summary>
    class PackageIndex
    {
        /// <summary>
        /// Package Entries
        /// </summary>
        public Dictionary<ulong, string> Entries = new Dictionary<ulong, string>();

        /// <summary>
        /// Loads Package Index File
        /// </summary>
        /// <param name="filePath">Path to Package file</param>
        public bool Load(string filePath)
        {
            using (BinaryReader reader = new BinaryReader(File.OpenRead(filePath)))
            {
                // Check Magic
                if (reader.ReadInt32() != 0x20494E57)
                {
                    return false;
                }
                else
                {
                    // Version / Count
                    int version = reader.ReadInt16();
                    int count = reader.ReadInt32();

                    // Buffer Sizes
                    int compressedSize = reader.ReadInt32();
                    int decompressedSize = reader.ReadInt32();

                    // Decode the Buffer
                    byte[] buffer = LZ4Codec.Decode(reader.ReadBytes(compressedSize), 0, compressedSize, decompressedSize);

                    // Read the package
                    using (BinaryReader internalReader = new BinaryReader(new MemoryStream(buffer)))
                        // Loop Count
                        for (int i = 0; i < count; i++)
                            // Read ID and String
                            Entries[internalReader.ReadUInt64() & 0xFFFFFFFFFFFFFFF] = ReadNullTerminatedString(internalReader);
                }
            }

            return true;
        }

        /// <summary>
        /// Saves compressed Package Index file
        /// </summary>
        /// <param name="filePath">Path to save Package file to</param>
        public void Save(string filePath)
        {
            using (var fileWriter = new BinaryWriter(File.Create(filePath)))
            {
                // Write Magic
                fileWriter.Write(0x20494E57);

                // Write Version / Count
                fileWriter.Write((short)1);
                fileWriter.Write(Entries.Count);

                // Create compressed output
                using (var compressedWriter = new BinaryWriter(new MemoryStream()))
                {
                    // Write Each Entry with ID and String
                    foreach (var entry in Entries)
                    {
                        compressedWriter.Write(entry.Key);
                        compressedWriter.Write(Encoding.UTF8.GetBytes(entry.Value));
                        compressedWriter.Write((byte)0);
                    }

                    // Get Length
                    int decompressedSize = (int)compressedWriter.BaseStream.Length;

                    // Compress Buffer
                    byte[] buffer = LZ4Codec.EncodeHC(((MemoryStream)compressedWriter.BaseStream).ToArray(), 0, decompressedSize);

                    // Write Lengths
                    fileWriter.Write(buffer.Length);
                    fileWriter.Write(decompressedSize);

                    // Write compressed buffer
                    fileWriter.Write(buffer);
                }

            }
        }

        /// <summary>
        /// Reads a string terminated by a null byte
        /// </summary>
        /// <returns>Read String</returns>
        private static string ReadNullTerminatedString(BinaryReader br, int maxSize = -1)
        {
            // Create String Builder
            StringBuilder str = new StringBuilder();
            // Current Byte Read
            int byteRead;
            // Size of String
            int size = 0;
            // Loop Until we hit terminating null character
            while ((byteRead = br.BaseStream.ReadByte()) != 0x0 && size++ != maxSize)
                str.Append(Convert.ToChar(byteRead));
            // Ship back Result
            return str.ToString();
        }
    }
}
