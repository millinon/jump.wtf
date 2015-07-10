<?hh

class rate_limiter
{

	private Memcached $mem;
	
	public function __construct(string $host){
		$this->mem = new Memcached('bans');

	}

}


