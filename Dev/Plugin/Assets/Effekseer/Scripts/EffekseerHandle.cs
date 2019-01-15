using UnityEngine;

namespace Effekseer
{
	/// <summary xml:lang="ja">
	/// A instance handle of played effect
	/// </summary>
	/// <summary xml:lang="ja">
	/// �Đ������G�t�F�N�g�̃C���X�^���X�n���h��
	/// </summary>
	public struct EffekseerHandle
	{
		internal int m_handle;

		public EffekseerHandle(int handle = -1)
		{
			m_handle = handle;
		}

		/// <summary>
		/// Don't touch it!!
		/// </summary>
		public void UpdateHandle(float deltaFrame)
		{
			Plugin.EffekseerUpdateHandle(m_handle, deltaFrame);
		}
		
		/// <summary xml:lang="en">
		/// Stops the played effect.
		/// All nodes will be destroyed.
		/// </summary>
		/// <summary xml:lang="ja">
		/// �G�t�F�N�g���~����
		/// �S�ẴG�t�F�N�g���u���ɏ�����
		/// </summary>
		public void Stop()
		{
			Plugin.EffekseerStopEffect(m_handle);
		}
	
		/// <summary xml:lang="en">
		/// Stops the root node of the played effect.
		/// The root node will be destroyed. Then children also will be destroyed by their lifetime.
		/// </summary>
		/// <summary xml:lang="ja">
		/// �Đ����̃G�t�F�N�g�̃��[�g�m�[�h�������~
		/// ���[�g�m�[�h���폜�������ƂŎq�m�[�h��������~��������ŏ��X�ɏ�����
		/// </summary>
		public void StopRoot()
		{
			Plugin.EffekseerStopRoot(m_handle);
		}
	
		/// <summary xml:lang="en">
		/// Sets the effect location
		/// </summary>
		/// <summary xml:lang="ja">
		/// �G�t�F�N�g�̈ʒu��ݒ�
		/// </summary>
		/// <param name="location">�ʒu</param>
		public void SetLocation(Vector3 location)
		{
			Plugin.EffekseerSetLocation(m_handle, location.x, location.y, location.z);
		}
	
		/// <summary xml:lang="en">
		/// Sets the effect rotation
		/// </summary>
		/// <summary xml:lang="ja">
		/// �G�t�F�N�g�̉�]��ݒ�
		/// </summary>
		/// <param name="rotation">��]</param>
		public void SetRotation(Quaternion rotation)
		{
			Vector3 axis;
			float angle;
			rotation.ToAngleAxis(out angle, out axis);
			Plugin.EffekseerSetRotation(m_handle, axis.x, axis.y, axis.z, angle * Mathf.Deg2Rad);
		}
	
		/// <summary xml:lang="en">
		/// Sets the effect scale
		/// </summary>
		/// <summary xml:lang="ja">
		/// �G�t�F�N�g�̊g�k��ݒ�
		/// </summary>
		/// <param name="scale">�g�k</param>
		public void SetScale(Vector3 scale)
		{
			Plugin.EffekseerSetScale(m_handle, scale.x, scale.y, scale.z);
		}

		/// <summary xml:lang="en">
		/// Specify the color of overall effect.
		/// </summary>
		/// <summary xml:lang="ja">
		/// �G�t�F�N�g�S�̂̐F���w�肷��B
		/// </summary>
		/// <param name="color">Color</param>
		public void SetAllColor(Color color)
		{
			Plugin.EffekseerSetAllColor(m_handle, (byte)(color.r * 255), (byte)(color.g * 255), (byte)(color.b * 255), (byte)(color.a * 255));
		}
	
		/// <summary xml:lang="en">
		/// Sets the effect target location
		/// </summary>
		/// <summary xml:lang="ja">
		/// �G�t�F�N�g�̃^�[�Q�b�g�ʒu��ݒ�
		/// </summary>
		/// <param name="targetLocation">�^�[�Q�b�g�ʒu</param>
		public void SetTargetLocation(Vector3 targetLocation)
		{
			Plugin.EffekseerSetTargetLocation(m_handle, targetLocation.x, targetLocation.y, targetLocation.z);
		}

		/// <summary xml:lang="en">
		/// Pausing the effect
		/// <para>true:  It will update on Update()</para>
		/// <para>false: It will not update on Update()</para>
		/// </summary>
		/// <summary xml:lang="ja">
		/// �|�[�Y�ݒ�
		/// <para>true:  ��~���BUpdate�ōX�V���Ȃ�</para>
		/// <para>false: �Đ����BUpdate�ōX�V����</para>
		/// </summary>
		public bool paused
		{
			set {
				Plugin.EffekseerSetPaused(m_handle, value);
			}
			get {
				return Plugin.EffekseerGetPaused(m_handle);
			}
		}
	
		/// <summary xml:lang="en">
		/// Showing the effect
		/// <para>true:  It will be rendering.</para>
		/// <para>false: It will not be rendering.</para>
		/// </summary>
		/// <summary xml:lang="ja">
		/// �\���ݒ�
		/// <para>true:  �\��ON�BDraw�ŕ`�悷��</para>
		/// <para>false: �\��OFF�BDraw�ŕ`�悵�Ȃ�</para>
		/// </summary>
		public bool shown
		{
			set {
				Plugin.EffekseerSetShown(m_handle, value);
			}
			get {
				return Plugin.EffekseerGetShown(m_handle);
			}
		}

		/// <summary xml:lang="en">
		/// Playback speed
		/// </summary>
		/// <summary xml:lang="ja">
		/// �Đ����x
		/// </summary>
		public float speed
		{
			set
			{
				Plugin.EffekseerSetSpeed(m_handle, value);
			}
			get
			{
				return Plugin.EffekseerGetSpeed(m_handle);
			}
		}
	
		/// <summary xml:lang="ja">
		/// Whether the effect instance is enabled<br/>
		/// <para>true:  enabled</para>
		/// <para>false: disabled</para>
		/// </summary>
		/// <summary xml:lang="ja">
		/// �C���X�^���X�n���h�����L�����ǂ���<br/>
		/// <para>true:  �L��</para>
		/// <para>false: ����</para>
		/// </summary>
		public bool enabled
		{
			get {
				return m_handle >= 0;
			}
		}
	
		/// <summary xml:lang="en">
		/// Existing state
		/// <para>true:  It's existed.</para>
		/// <para>false: It isn't existed or stopped.</para>
		/// </summary>
		/// <summary xml:lang="ja">
		/// �G�t�F�N�g�̃C���X�^���X�����݂��Ă��邩�ǂ���
		/// <para>true:  ���݂��Ă���</para>
		/// <para>false: �Đ��I���Ŕj���B��������Stop�Œ�~���ꂽ</para>
		/// </summary>
		public bool exists
		{
			get {
				return Plugin.EffekseerExists(m_handle);
			}
		}
	}
}