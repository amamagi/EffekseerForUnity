﻿using System;
using UnityEngine;

#if UNITY_EDITOR
using UnityEditor;

namespace Effekseer.Editor
{
	using Internal;

	[CustomEditor(typeof(EffekseerEffectAsset))]
	public class EffekseerEffectAssetEditor : UnityEditor.Editor
	{
		bool textureVisible = true;
		bool soundVisible = true;
		bool modelVisible = true;

		public override void OnInspectorGUI()
		{
			var asset = target as EffekseerEffectAsset;
			
			EditorGUILayout.LabelField("Data Size", asset.efkBytes.Length.ToString() + " bytes");

			textureVisible = EditorGUILayout.Foldout(textureVisible, "Texture Resources: " + asset.textureResources.Length);
			if (textureVisible) {
				EditorGUI.indentLevel++;
				foreach (var res in asset.textureResources) {
					if (EffekseerTextureResource.InspectorField(res)) {
						EditorUtility.SetDirty(asset);
					}
				}
				EditorGUI.indentLevel--;
			}
			
			soundVisible = EditorGUILayout.Foldout(soundVisible, "Sound Resources: " + asset.soundResources.Length);
			if (soundVisible) {
				EditorGUI.indentLevel++;
				foreach (var res in asset.soundResources) {
					if (EffekseerSoundResource.InspectorField(res)) {
						EditorUtility.SetDirty(asset);
					}
				}
				EditorGUI.indentLevel--;
			}
			
			modelVisible = EditorGUILayout.Foldout(modelVisible, "Model Resources: " + asset.modelResources.Length);
			if (modelVisible) {
				EditorGUI.indentLevel++;
				foreach (var res in asset.modelResources) {
					if (EffekseerModelResource.InspectorField(res)) {
						EditorUtility.SetDirty(asset);
					}
				}
				EditorGUI.indentLevel--;
			}
		}
	}
}

#endif