using UnityEngine;
using System.Collections;

public class VideoController : MonoBehaviour {
    
    public Camera camera;
    public int rotateSpeed = 100;

    private Texture2D[] frames;

    void Start()
    {
    }

    void Update()
    {
        var v3 = new Vector3(Input.GetAxis("Vertical"), Input.GetAxis("Horizontal"), 0.0f);
        camera.transform.Rotate(v3 * rotateSpeed * Time.deltaTime);
    }
}
