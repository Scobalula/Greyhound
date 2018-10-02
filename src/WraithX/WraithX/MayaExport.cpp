#include "stdafx.h"

// The class we are implementing
#include "MayaExport.h"

// We need the textwriter, hashing, and filesystems classes
#include "TextWriter.h"
#include "Hashing.h"
#include "FileSystems.h"

// Generic maya file settings
static const char* GenericSettings = "createNode transform -s -n \"persp\";\n\tsetAttr \".v\" no;\n\tsetAttr \".t\" -type \"double3\" 48.186233840145825 37.816674066853686 41.0540421364379 ;\n\tsetAttr \".r\" -type \"double3\" -29.738352729603015 49.400000000000432 0 ;\ncreateNode camera -s -n \"perspShape\" -p \"persp\";\n\tsetAttr -k off \".v\" no;\n\tsetAttr \".fl\" 34.999999999999993;\n\tsetAttr \".fcp\" 10000;\n\tsetAttr \".coi\" 73.724849603665149;\n\tsetAttr \".imn\" -type \"string\" \"persp\";\n\tsetAttr \".den\" -type \"string\" \"persp_depth\";\n\tsetAttr \".man\" -type \"string\" \"persp_mask\";\n\tsetAttr \".hc\" -type \"string\" \"viewSet -p %camera\";\ncreateNode transform -s -n \"top\";\n\tsetAttr \".v\" no;\n\tsetAttr \".t\" -type \"double3\" 0 100.1 0 ;\n\tsetAttr \".r\" -type \"double3\" -89.999999999999986 0 0 ;\ncreateNode camera -s -n \"topShape\" -p \"top\";\n\tsetAttr -k off \".v\" no;\n\tsetAttr \".rnd\" no;\n\tsetAttr \".coi\" 100.1;\n\tsetAttr \".ow\" 30;\n\tsetAttr \".imn\" -type \"string\" \"top\";\n\tsetAttr \".den\" -type \"string\" \"top_depth\";\n\tsetAttr \".man\" -type \"string\" \"top_mask\";\n\tsetAttr \".hc\" -type \"string\" \"viewSet -t %camera\";\n\tsetAttr \".o\" yes;\ncreateNode transform -s -n \"front\";\n\tsetAttr \".v\" no;\n\tsetAttr \".t\" -type \"double3\" 0 0 100.1 ;\ncreateNode camera -s -n \"frontShape\" -p \"front\";\n\tsetAttr -k off \".v\" no;\n\tsetAttr \".rnd\" no;\n\tsetAttr \".coi\" 100.1;\n\tsetAttr \".ow\" 30;\n\tsetAttr \".imn\" -type \"string\" \"front\";\n\tsetAttr \".den\" -type \"string\" \"front_depth\";\n\tsetAttr \".man\" -type \"string\" \"front_mask\";\n\tsetAttr \".hc\" -type \"string\" \"viewSet -f %camera\";\n\tsetAttr \".o\" yes;\ncreateNode transform -s -n \"side\";\n\tsetAttr \".v\" no;\n\tsetAttr \".t\" -type \"double3\" 100.1 0 0 ;\n\tsetAttr \".r\" -type \"double3\" 0 89.999999999999986 0 ;\ncreateNode camera -s -n \"sideShape\" -p \"side\";\n\tsetAttr -k off \".v\" no;\n\tsetAttr \".rnd\" no;\n\tsetAttr \".coi\" 100.1;\n\tsetAttr \".ow\" 30;\n\tsetAttr \".imn\" -type \"string\" \"side\";\n\tsetAttr \".den\" -type \"string\" \"side_depth\";\n\tsetAttr \".man\" -type \"string\" \"side_mask\";\n\tsetAttr \".hc\" -type \"string\" \"viewSet -s %camera\";\n\tsetAttr \".o\" yes;\ncreateNode lightLinker -n \"lightLinker1\";\n\tsetAttr -s 9 \".lnk\";\n\tsetAttr -s 9 \".slnk\";\ncreateNode displayLayerManager -n \"layerManager\";\ncreateNode displayLayer -n \"defaultLayer\";\ncreateNode renderLayerManager -n \"renderLayerManager\";\ncreateNode renderLayer -n \"defaultRenderLayer\";\n\tsetAttr \".g\" yes;\ncreateNode script -n \"sceneConfigurationScriptNode\";\n\tsetAttr \".b\" -type \"string\" \"playbackOptions -min 1 -max 24 -ast 1 -aet 48 \";\n\tsetAttr \".st\" 6;\nselect -ne :time1;\n\tsetAttr \".o\" 1;\nselect -ne :renderPartition;\n\tsetAttr -s 2 \".st\";\nselect -ne :renderGlobalsList1;\nselect -ne :defaultShaderList1;\n\tsetAttr -s 2 \".s\";\nselect -ne :postProcessList1;\n\tsetAttr -s 2 \".p\";\nselect -ne :lightList1;\nselect -ne :initialShadingGroup;\n\tsetAttr \".ro\" yes;\nselect -ne :initialParticleSE;\n\tsetAttr \".ro\" yes;\nselect -ne :hardwareRenderGlobals;\n\tsetAttr \".ctrs\" 256;\n\tsetAttr \".btrs\" 512;\nselect -ne :defaultHardwareRenderGlobals;\n\tsetAttr \".fn\" -type \"string\" \"im\";\n\tsetAttr \".res\" -type \"string\" \"ntsc_4d 646 485 1.333\";\nselect -ne :ikSystem;\n\tsetAttr -s 4 \".sol\";\nconnectAttr \":defaultLightSet.msg\" \"lightLinker1.lnk[0].llnk\";\nconnectAttr \":initialShadingGroup.msg\" \"lightLinker1.lnk[0].olnk\";\nconnectAttr \":defaultLightSet.msg\" \"lightLinker1.lnk[1].llnk\";\nconnectAttr \":initialParticleSE.msg\" \"lightLinker1.lnk[1].olnk\";\nconnectAttr \":defaultLightSet.msg\" \"lightLinker1.slnk[0].sllk\";\nconnectAttr \":initialShadingGroup.msg\" \"lightLinker1.slnk[0].solk\";\nconnectAttr \":defaultLightSet.msg\" \"lightLinker1.slnk[1].sllk\";\nconnectAttr \":initialParticleSE.msg\" \"lightLinker1.slnk[1].solk\";\nconnectAttr \"layerManager.dli[0]\" \"defaultLayer.id\";\nconnectAttr \"renderLayerManager.rlmi[0]\" \"defaultRenderLayer.rlid\";\nconnectAttr \"lightLinker1.msg\" \":lightList1.ln\" -na;\n";

// Sort a vector by it's id value (For simple weight sorting)
bool SortByWeightValue(uint32_t i, uint32_t j) { return (i < j); }

void Maya::ExportMaya(const WraithModel& Model, const std::string& FileName)
{
	// Hash the name
	auto HashName = Hashing::HashCRC32String(Model.AssetName);
	// Prepare to write the model file
	{
		// Create a new writer
		auto Writer = TextWriter();
		// Open the model file
		Writer.Create(FileName);
		// Set buffer
		Writer.SetWriteBuffer(0x100000);
		// Write the header
		Writer.WriteLine("//Maya ASCII 8.5 scene\n\n// Wraith - Game extraction tools");
		Writer.WriteLine("// Please credit DTZxPorter for using it!\n");
		// Output maya header
		Writer.WriteLine("requires maya \"8.5\";\ncurrentUnit -l centimeter -a degree -t film;\nfileInfo \"application\" \"maya\";\nfileInfo \"product\" \"Maya Unlimited 8.5\";\nfileInfo \"version\" \"8.5\";\nfileInfo \"cutIdentifier\" \"200612162224-692032\";");
		// Output generics
		Writer.Write(GenericSettings);
		// Create model node
		Writer.WriteLineFmt("createNode transform -n \"%s\";", Model.AssetName.c_str());
		// Set some attribute
		Writer.WriteLine("setAttr \".ove\" yes;");
		// The submesh index buffer
		uint32_t SubmeshIndex = 0;
		// Iterate over all submeshes
		for (auto& Submesh : Model.Submeshes)
		{
			// Create the submesh
			Writer.WriteLineFmt("createNode transform -n \"WraithMesh_%s_%02d\" -p \"%s\";", HashName.c_str(), SubmeshIndex, Model.AssetName.c_str());
			// Basic RP and SP values
			Writer.WriteLineFmt("setAttr \".rp\" -type \"double3\" 0.000000 0.000000 0.000000 ;\nsetAttr \".sp\" -type \"double3\" 0.000000 0.000000 0.000000 ;");
			// Create a Maya mesh
			Writer.WriteLineFmt("createNode mesh -n \"MeshShape_%d\" -p \"WraithMesh_%s_%02d\";", SubmeshIndex, HashName.c_str(), SubmeshIndex);
			// Setup the verticies
			Writer.WriteLine("setAttr -k off \".v\";\nsetAttr \".vir\" yes;\nsetAttr \".vif\" yes;");
			// Set count of components, 1, then UV count, same as vert count
			Writer.WriteLineFmt("setAttr \".iog[0].og[0].gcl\" -type \"componentList\" 1 \"f[0:%d]\";", Submesh.VertexCount());
			// Set UV layer, count and index
			Writer.WriteLineFmt("setAttr \".uvst[0].uvsn\" -type \"string\" \"map1\";\nsetAttr -s %d \".uvst[0].uvsp\";", Submesh.VertexCount());

			// Prepare to output the UVs
			if (Submesh.VertexCount() == 1)
				Writer.Write("setAttr \".uvst[0].uvsp[0]\" -type \"float2\"");
			else
				Writer.WriteFmt("setAttr \".uvst[0].uvsp[0:%d]\" -type \"float2\"", (Submesh.VertexCount() - 1));
			// Iterate and write
			for (auto& Vertex : Submesh.Verticies)
			{
				// Write it, for now, we are only applying the initial UV layer
				Writer.WriteFmt(" %f %f", Vertex.UVLayers[0].U, (1 - Vertex.UVLayers[0].V));
			}
			// Close
			Writer.WriteLine(";");

			// Set color UVs
			Writer.WriteLine("setAttr \".cuvs\" -type \"string\" \"map1\";\nsetAttr \".dcol\" yes;\nsetAttr \".dcc\" -type \"string\" \"Ambient+Diffuse\";\nsetAttr \".ccls\" -type \"string\" \"colorSet1\";\nsetAttr \".clst[0].clsn\" -type \"string\" \"colorSet1\";");
			// Setup color count
			Writer.WriteLineFmt("setAttr -s %d \".clst[0].clsp\";", (Submesh.FacesCount() * 3));

			// Prepare to output the Colors
			Writer.WriteFmt("setAttr \".clst[0].clsp[0:%d]\"", ((Submesh.FacesCount() * 3) - 1));
			// Iterate and output
			for (auto& Face : Submesh.Faces)
			{
				// Output (3 instances per face)
				Writer.WriteFmt(" %f %f %f %f %f %f %f %f %f %f %f %f",
					// The the verts, 3->2->1
					Submesh.Verticies[Face.Index3].Color[0] / 255.0f, Submesh.Verticies[Face.Index3].Color[1] / 255.0f, Submesh.Verticies[Face.Index3].Color[2] / 255.0f, Submesh.Verticies[Face.Index3].Color[3] / 255.0f,
					Submesh.Verticies[Face.Index2].Color[0] / 255.0f, Submesh.Verticies[Face.Index2].Color[1] / 255.0f, Submesh.Verticies[Face.Index2].Color[2] / 255.0f, Submesh.Verticies[Face.Index2].Color[3] / 255.0f,
					Submesh.Verticies[Face.Index1].Color[0] / 255.0f, Submesh.Verticies[Face.Index1].Color[1] / 255.0f, Submesh.Verticies[Face.Index1].Color[2] / 255.0f, Submesh.Verticies[Face.Index1].Color[3] / 255.0f
				);
			}
			// Close
			Writer.WriteLine(";");

			// Set vertex position section and count
			Writer.WriteLineFmt("setAttr \".covm[0]\"  0 1 1;\nsetAttr \".cdvm[0]\"  0 1 1;\nsetAttr -s %d \".vt\";", Submesh.VertexCount());

			// Prepare to output the Positions
			if (Submesh.VertexCount() == 1)
				Writer.Write("setAttr \".vt[0]\"");
			else
				Writer.WriteFmt("setAttr \".vt[0:%d]\"", (Submesh.VertexCount() - 1));
			// Iterate and write
			for (auto& Vertex : Submesh.Verticies)
			{
				// Output positions
				Writer.WriteFmt(" %f %f %f", Vertex.Position.X, Vertex.Position.Y, Vertex.Position.Z);
			}
			// Close
			Writer.WriteLine(";");

			// Setup edges count
			Writer.WriteLineFmt("setAttr -s %d \".ed\";", (Submesh.FacesCount() * 3));

			// Prepare to output Edges
			Writer.WriteFmt("setAttr \".ed[0:%d]\"", ((Submesh.FacesCount() * 3) - 1));
			// Iterate and output
			for (auto& Face : Submesh.Faces)
			{
				// Output edge combinations, 3 per face 3->2 2->1 1->3
				Writer.WriteFmt(" %d %d 0 %d %d 0 %d %d 0", Face.Index3, Face.Index2, Face.Index2, Face.Index1, Face.Index1, Face.Index3);
			}
			// Close
			Writer.WriteLine(";");

			// Setup normal count
			Writer.WriteLineFmt("setAttr -s %d \".n\";", (Submesh.FacesCount() * 3));

			// Prepare to output the Normals
			Writer.WriteFmt("setAttr \".n[0:%d]\" -type \"float3\"", ((Submesh.FacesCount() * 3) - 1));
			// Iterate and output
			for (auto& Face : Submesh.Faces)
			{
				// Output normals, 3 per face (3 verts per face) 3, 2, 1
				Writer.WriteFmt(" %f %f %f %f %f %f %f %f %f",
					// The the verts, 3->2->1
					Submesh.Verticies[Face.Index3].Normal.X, Submesh.Verticies[Face.Index3].Normal.Y, Submesh.Verticies[Face.Index3].Normal.Z,
					Submesh.Verticies[Face.Index2].Normal.X, Submesh.Verticies[Face.Index2].Normal.Y, Submesh.Verticies[Face.Index2].Normal.Z,
					Submesh.Verticies[Face.Index1].Normal.X, Submesh.Verticies[Face.Index1].Normal.Y, Submesh.Verticies[Face.Index1].Normal.Z
					);
			}
			// Close
			Writer.WriteLine(";");

			// Prepare to output the Faces
			if (Submesh.FacesCount() == 1)
				Writer.WriteFmt("setAttr -s %d \".fc[0]\" -type \"polyFaces\"", Submesh.FacesCount());
			else
				Writer.WriteFmt("setAttr -s %d \".fc[0:%d]\" -type \"polyFaces\"", Submesh.FacesCount(), (Submesh.FacesCount() - 1));
			// Iterate and output
			{
				// Buffer for face index
				uint32_t FaceIndex = 0;
				// Iterate
				for (auto& Face : Submesh.Faces)
				{
					// Output face connections, 3 per face, index, uv, index
					Writer.WriteFmt(" f 3 %d %d %d mu 0 3 %d %d %d mc 0 3 %d %d %d",
						// The indicies 1->2->3
						FaceIndex, (FaceIndex + 1), (FaceIndex + 2),
						Face.Index3, Face.Index2, Face.Index1,
						FaceIndex, (FaceIndex + 1), (FaceIndex + 2)
						);
					// Advance
					FaceIndex += 3;
				}
				// Close
				Writer.WriteLine(";");
			}
			// End the submesh
			Writer.WriteLine("setAttr \".cd\" -type \"dataPolyComponent\" Index_Data Edge 0 ;\nsetAttr \".cvd\" -type \"dataPolyComponent\" Index_Data Vertex 0 ;\nsetAttr \".hfd\" -type \"dataPolyComponent\" Index_Data Face 0 ;");
			// Advance
			SubmeshIndex++;
		}
		// Spacer for materials
		Writer.NewLine();
		// A unique list of material indicies
		std::unordered_set<int32_t> MaterialIndicies;
		// Iterate over submeshes
		for (auto& Submesh : Model.Submeshes)
		{
			// Grab a reference to the material, if the submesh actually references one or not
			if (MaterialIndicies.insert(Submesh.MaterialIndicies[0]).second)
			{
				// We can use this index
				const WraithMaterial& Material = (Submesh.MaterialIndicies[0] > -1) ? Model.Materials[Submesh.MaterialIndicies[0]] : WraithMaterial::DefaultMaterial;
				// Make the material, and set common config
				Writer.WriteLineFmt("createNode shadingEngine -n \"%sSG\";\n	setAttr \".ihi\" 0;\n	setAttr \".ro\" yes;", Material.MaterialName.c_str());
				// Set material info
				Writer.WriteLineFmt("createNode materialInfo -n \"%sMI\";\r\ncreateNode phong -n \"%s\";", Material.MaterialName.c_str(), Material.MaterialName.c_str());
				// Set color
				Writer.WriteLine("	setAttr \".ambc\" -type \"float3\" 1 1 1 ;");
				// Check if we have a diffuse map, if so, assign the file
				if (!Strings::IsNullOrWhiteSpace(Material.DiffuseMapName))
				{
					// We can assign the file
					Writer.WriteLineFmt("createNode file -n \"%sFILE\";\n	setAttr \".ftn\" -type \"string\" \"%s\";", Material.MaterialName.c_str(), Material.DiffuseMapName.c_str());
				}
				// Create a texture
				Writer.WriteLineFmt("createNode place2dTexture -n \"%sP2DT\";\n", Material.MaterialName.c_str());
			}
		}
		// The light connection offset
		uint32_t LightConnectionIndex = 2;
		// Iterate over the used materials
		for (auto& MaterialIndex : MaterialIndicies)
		{
			// Grab material reference
			const WraithMaterial& Material = (MaterialIndex > -1) ? Model.Materials[MaterialIndex] : WraithMaterial::DefaultMaterial;
			// Connect the material connections
			Writer.WriteLineFmt(
				// The formatted lines for the connections
				"connectAttr \":defaultLightSet.msg\" \"lightLinker1.lnk[%d].llnk\";\n"
				"connectAttr \"%sSG.msg\" \"lightLinker1.lnk[%d].olnk\";\n"
				"connectAttr \":defaultLightSet.msg\" \"lightLinker1.slnk[%d].sllk\";\n"
				"connectAttr \"%sSG.msg\" \"lightLinker1.slnk[%d].solk\";\n"
				"connectAttr \"%s.oc\" \"%sSG.ss\";\n"
				"connectAttr \"%sSG.msg\" \"%sMI.sg\";\n"
				"connectAttr \"%s.msg\" \"%sMI.m\";",
				// The data for each line of a connection
				LightConnectionIndex,
				Material.MaterialName.c_str(), LightConnectionIndex,
				LightConnectionIndex,
				Material.MaterialName.c_str(), LightConnectionIndex,
				Material.MaterialName.c_str(), Material.MaterialName.c_str(),
				Material.MaterialName.c_str(), Material.MaterialName.c_str(),
				Material.MaterialName.c_str(), Material.MaterialName.c_str()
				);
			// Connect material to file if available
			if (!Strings::IsNullOrWhiteSpace(Material.DiffuseMapName))
			{
				// Connect to file
				Writer.WriteLineFmt(
					// The formatted lines for connections
					"connectAttr \"%sFILE.msg\" \"%sMI.t\" -na;\n"
					"connectAttr \"%sFILE.oc\" \"%s.c\";\n"
					"connectAttr \"%sP2DT.c\" \"%sFILE.c\";\n"
					"connectAttr \"%sP2DT.tf\" \"%sFILE.tf\";\n"
					"connectAttr \"%sP2DT.rf\" \"%sFILE.rf\";\n"
					"connectAttr \"%sP2DT.mu\" \"%sFILE.mu\";\n"
					"connectAttr \"%sP2DT.mv\" \"%sFILE.mv\";\n"
					"connectAttr \"%sP2DT.s\" \"%sFILE.s\";\n"
					"connectAttr \"%sP2DT.wu\" \"%sFILE.wu\";\n"
					"connectAttr \"%sP2DT.wv\" \"%sFILE.wv\";\n"
					"connectAttr \"%sP2DT.re\" \"%sFILE.re\";\n"
					"connectAttr \"%sP2DT.of\" \"%sFILE.of\";\n"
					"connectAttr \"%sP2DT.r\" \"%sFILE.ro\";\n"
					"connectAttr \"%sP2DT.n\" \"%sFILE.n\";\n"
					"connectAttr \"%sP2DT.vt1\" \"%sFILE.vt1\";\n"
					"connectAttr \"%sP2DT.vt2\" \"%sFILE.vt2\";\n"
					"connectAttr \"%sP2DT.vt3\" \"%sFILE.vt3\";\n"
					"connectAttr \"%sP2DT.vc1\" \"%sFILE.vc1\";\n"
					"connectAttr \"%sP2DT.o\" \"%sFILE.uv\";\n"
					"connectAttr \"%sP2DT.ofs\" \"%sFILE.fs\";\n",
					// The data for each line of a connection
					Material.MaterialName.c_str(), Material.MaterialName.c_str(),
					Material.MaterialName.c_str(), Material.MaterialName.c_str(),
					Material.MaterialName.c_str(), Material.MaterialName.c_str(),
					Material.MaterialName.c_str(), Material.MaterialName.c_str(),
					Material.MaterialName.c_str(), Material.MaterialName.c_str(),
					Material.MaterialName.c_str(), Material.MaterialName.c_str(),
					Material.MaterialName.c_str(), Material.MaterialName.c_str(),
					Material.MaterialName.c_str(), Material.MaterialName.c_str(),
					Material.MaterialName.c_str(), Material.MaterialName.c_str(),
					Material.MaterialName.c_str(), Material.MaterialName.c_str(),
					Material.MaterialName.c_str(), Material.MaterialName.c_str(),
					Material.MaterialName.c_str(), Material.MaterialName.c_str(),
					Material.MaterialName.c_str(), Material.MaterialName.c_str(),
					Material.MaterialName.c_str(), Material.MaterialName.c_str(),
					Material.MaterialName.c_str(), Material.MaterialName.c_str(),
					Material.MaterialName.c_str(), Material.MaterialName.c_str(),
					Material.MaterialName.c_str(), Material.MaterialName.c_str(),
					Material.MaterialName.c_str(), Material.MaterialName.c_str(),
					Material.MaterialName.c_str(), Material.MaterialName.c_str(),
					Material.MaterialName.c_str(), Material.MaterialName.c_str()
					);
			}
			// Connect the shaders
			Writer.WriteLineFmt(
				// The formatted lines for the connections
				"connectAttr \"%sSG.pa\" \":renderPartition.st\" -na;\n"
				"connectAttr \"%s.msg\" \":defaultShaderList1.s\" -na;\n"
				"connectAttr \"%sP2DT.msg\" \":defaultRenderUtilityList1.u\" -na;\n"
				"connectAttr \"%sFILE.msg\" \":defaultTextureList1.tx\" -na;\n",
				// The data for each line of a connection
				Material.MaterialName.c_str(),
				Material.MaterialName.c_str(),
				Material.MaterialName.c_str(),
				Material.MaterialName.c_str()
				);
			// Increase
			LightConnectionIndex++;
		}
		// A buffer for material offsets
		uint32_t SubmeshMaterialOffset = 0;
		// Iterate and connect materials
		for (auto& Submesh : Model.Submeshes)
		{
			// Grab reference to the material
			const WraithMaterial& Material = (Submesh.MaterialIndicies[0] > -1) ? Model.Materials[Submesh.MaterialIndicies[0]] : WraithMaterial::DefaultMaterial;
			// Connect it
			Writer.WriteLineFmt(
				"connectAttr \"%sSG.mwc\" \"MeshShape_%d.iog.og[0].gco\";\n"
				"connectAttr \"MeshShape_%d.iog\" \"%sSG.dsm\" -na;",
				Material.MaterialName.c_str(), SubmeshMaterialOffset,
				SubmeshMaterialOffset, Material.MaterialName.c_str()
				);
			// Advance
			SubmeshMaterialOffset++;
		}
		// Create joints
		Writer.WriteLine("createNode transform -n \"Joints\";\nsetAttr \".ove\" yes;\n");
		// Iterate over bones
		for (auto& Bone : Model.Bones)
		{
			// If the parent is -1, connect to joints
			if (Bone.BoneParent == -1)
			{
				// Connect to joints
				Writer.WriteLineFmt("createNode joint -n \"%s\" -p \"Joints\";", Bone.TagName.c_str());
			}
			else
			{
				// Connect to parent
				Writer.WriteLineFmt("createNode joint -n \"%s\" -p \"%s\";", Bone.TagName.c_str(), Model.Bones[Bone.BoneParent].TagName.c_str());
			}
			// Grab local rotation as euler
			Vector3 LocalRotation = Bone.LocalRotation.ToEulerAngles();
			// Lock weights, then apply bone information
			Writer.WriteLineFmt(
				// The formatted lines
				"addAttr -ci true -sn \"liw\" -ln \"lockInfluenceWeights\" -bt \"lock\" -min 0 -max 1 -at \"bool\";\nsetAttr \".uoc\" yes;\nsetAttr \".ove\" yes;\n"
				"setAttr \".t\" -type \"double3\"  %f %f %f ;\n"
				"setAttr \".mnrl\" -type \"double3\" -360 -360 -360 ;\nsetAttr \".mxrl\" -type \"double3\" 360 360 360 ;\nsetAttr \".radi\"   0.50;\n"
				"setAttr \".jo\" -type \"double3\" %f %f %f;\n"
				"setAttr \".scale\" -type \"double3\" %f %f %f;\n",
				// The data for the formatted lines
				Bone.LocalPosition.X, Bone.LocalPosition.Y, Bone.LocalPosition.Z,
				LocalRotation.X, LocalRotation.Y, LocalRotation.Z,
				Bone.BoneScale.X, Bone.BoneScale.Y, Bone.BoneScale.Z
				);
		}
	}
	// Start the bind file
	{
		// Create a new writer
		auto Writer = TextWriter();
		// Open the bind file, created from the name of the main file
		Writer.Create(FileSystems::CombinePath(FileSystems::GetDirectoryName(FileName), FileSystems::GetFileNameWithoutExtension(FileName) + "_BIND.mel"));
		// Set buffer
		Writer.SetWriteBuffer(0x100000);
		// Write the header
		Writer.WriteLine("/*\n* Advanced weighting script X\n* Wraith - Game extraction tools\n*/\n");
		// The submesh index buffer
		uint32_t SubmeshIndex = 0;
		// Iterate over all submeshes
		for (auto& Submesh : Model.Submeshes)
		{
			// Output function start then select mesh
			Writer.WriteLineFmt(
				"global proc WraithMesh_%s_%02d_BindFunc()\n{\n"
				"   select -r WraithMesh_%s_%02d;",
				HashName.c_str(), SubmeshIndex,
				HashName.c_str(), SubmeshIndex
				);
			// A list of simple weights per bone
			std::unordered_map<std::string, std::vector<uint32_t>> SimpleBoneWeights;
			// A list of complex weights per bone
			std::unordered_map<std::string, std::unordered_map<float, std::vector<uint32_t>>> ComplexBoneWeights;
			// A list of unique bone names
			std::unordered_set<std::string> UniqueBoneNames;
			// The vertex index buffer
			uint32_t VertexIndex = 0;
			// Prepare to optimize and sort weights
			for (auto& Vertex : Submesh.Verticies)
			{
				// Weight index
				uint32_t Index = 0;
				// Support as many weights as we have
				for (auto& Weight : Vertex.Weights)
				{
					// Calculate index
					auto BoneID = (Weight.BoneIndex == -1) ? 0 : Weight.BoneIndex;
					// Check weight index
					if (Index == 0)
					{
						// Simple weight just add it's index
						SimpleBoneWeights[Model.Bones[BoneID].TagName].push_back(VertexIndex);
					}
					else
					{
						// Complex weight
						ComplexBoneWeights[Model.Bones[BoneID].TagName][Weight.Weight].push_back(VertexIndex);
					}
					// Add to unique names
					UniqueBoneNames.insert(Model.Bones[BoneID].TagName);
					// Advance
					Index++;
				}
				// Advance
				VertexIndex++;
			}
			// Select required bones
			for (auto& Bone : UniqueBoneNames)
			{
				// Select
				Writer.WriteLineFmt("   select -add %s;", Bone.c_str());
			}
			// Prepare the cluster
			Writer.WriteLineFmt(
				"   newSkinCluster \"-toSelectedBones -mi 30 -omi true -dr 5.0 -rui false\";"
				"   string $clu = findRelatedSkinCluster(\"WraithMesh_%s_%02d\");\n",
				HashName.c_str(), SubmeshIndex
				);
			// Sort simple weights and output
			for (auto& SimpleWeights : SimpleBoneWeights)
			{
				// Sort then output
				std::sort(SimpleWeights.second.begin(), SimpleWeights.second.end(), SortByWeightValue);
				// Iterate and output
				Writer.WriteFmt("   skinPercent -tv %s 1.0 $clu", SimpleWeights.first.c_str());
				// Cache size
				auto WeightLen = SimpleWeights.second.size();
				// Iterate
				for (size_t i = 0; i < WeightLen; i++)
				{
					// Start the base
					Writer.WriteFmt(" WraithMesh_%s_%02d.vtx[%d", HashName.c_str(), SubmeshIndex, SimpleWeights.second[i]);
					// Buffer for ending
					size_t j = i;
					// Search for ending
					for (; j < WeightLen; j++)
					{
						// Ensure that j = (j - 1) + 1 and that we aren't the last one
						if ((j + 1) >= WeightLen || (SimpleWeights.second[j] + 1) != SimpleWeights.second[j + 1])
						{
							// Stop
							break;
						}
					}
					// Check
					if (i == j)
					{
						// Non-array
						Writer.Write("]");
					}
					else
					{
						// Array
						Writer.WriteFmt(":%d]", SimpleWeights.second[j]);
					}
					// Set
					i = j;
				}
				// Close
				Writer.WriteLine(";");
			}
			// Sort complex weights and output
			for (auto& ComplexWeights : ComplexBoneWeights)
			{
				// Iterate over weight values
				for (auto& WeightSet : ComplexWeights.second)
				{
					// Sort then output
					std::sort(WeightSet.second.begin(), WeightSet.second.end(), SortByWeightValue);
					// Start the segment
					Writer.WriteFmt("   skinPercent -tv %s %f $clu", ComplexWeights.first.c_str(), WeightSet.first);

					// Cache size
					auto WeightLen = WeightSet.second.size();
					// Iterate
					for (size_t i = 0; i < WeightLen; i++)
					{
						// Start the base
						Writer.WriteFmt(" WraithMesh_%s_%02d.vtx[%d", HashName.c_str(), SubmeshIndex, WeightSet.second[i]);
						// Buffer for ending
						size_t j = i;
						// Search for ending
						for (; j < WeightLen; j++)
						{
							// Ensure that j = (j - 1) + 1 and that we aren't the last one
							if ((j + 1) >= WeightLen || (WeightSet.second[j] + 1) != WeightSet.second[j + 1])
							{
								// Stop
								break;
							}
						}
						// Check
						if (i == j)
						{
							// Non-array
							Writer.Write("]");
						}
						else
						{
							// Array
							Writer.WriteFmt(":%d]", WeightSet.second[j]);
						}
						// Set
						i = j;
					}

					// End it
					Writer.WriteLine(";");
				}
			}
			// End function
			Writer.WriteLine("}\n");
			// Advance
			SubmeshIndex++;
		}
		// Run function
		Writer.WriteLine("global proc RunAdvancedScript()\n{");
		// Loop through submeshes once more and call
		for (size_t SubmeshIndex = 0; SubmeshIndex < Model.SubmeshCount(); SubmeshIndex++)
		{
			// Call it
			Writer.WriteLineFmt("   catch(WraithMesh_%s_%02d_BindFunc());", HashName.c_str(), SubmeshIndex);
		}
		// Close and output final data
		Writer.WriteLine("}\n\nglobal proc NamespacePurge()\n{\n   string $allNodes[] = `ls`;\n   for($node in $allNodes) {\n      string $buffer[];\n      tokenize $node \":\" $buffer;\n      string $newName = $buffer[size($buffer)-1];\n       catchQuiet(`rename $node $newName`);\n   }\n}\n\nprint(\"Currently binding the current model, please wait...\\n\");\nNamespacePurge();\nRunAdvancedScript();\nprint(\"The model has been binded.\\n\");\n");
	}
}