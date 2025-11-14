async function uploadFile(endpoint){
  const input = document.querySelector('#fileInput');
  const status = document.querySelector('#status');
  if(!input.files.length){ status.textContent='Please choose a file'; status.className='status error'; return; }
  const file = input.files[0];
  status.textContent='Uploading '+file.name+' ('+file.size+' bytes)...'; status.className='status';
  try{
    const res = await fetch(endpoint, {
      method:'POST',
      headers:{
        'Content-Type':'application/octet-stream',
        'X-Filename': encodeURIComponent(file.name)
      },
      body: file
    });
    const text = await res.text();
    if(res.status===201||res.status===200){
      let loc = res.headers.get('Location');
      if(loc){ status.innerHTML = 'Upload done ('+res.status+'). <a href="'+loc+'">'+(loc)+'</a>'; }
      else { status.textContent='Upload done ('+res.status+'). '+text; }
      status.className='status success';
    }else{
      status.textContent='Upload failed: '+res.status+' '+text; status.className='status error';
    }
  }catch(e){ status.textContent='Upload error: '+e; status.className='status error'; }
}
