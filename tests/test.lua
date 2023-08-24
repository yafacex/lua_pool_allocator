collectgarbage("collect")
local function dumpStat()
	local stats = alloc.getStat()
	for _,stat in ipairs(stats)do
		print(string.format("blockSize:%d,iCreate:%d,iFree:%d,iHitCreate:%d,iHitFree:%d,blockCount:%d,iChunkCount:%d",stat.blockSize,stat.iCreate,stat.iFree,stat.iHitCreate,stat.iHitFree,stat.blockCount,stat.iChunkCount))
	end
	print("totalMem(MB):",stats.cacheMem / 1024 / 1024)
end


local function test_mix()
	--这是一个不适合的情况，表A里放了太多数据（而且都是小table），
	--造成了极高的内存峰值，该算法的缺点是频繁复用小table，而无法回收。
	--所以需申请大量小table的时候不要存储，从而造成内存峰值后无法回收。
	local A = {}
	for k = 1,1000 do
		local out = {}
		A[k] = {}
		for i = 1,2048 do
			local all = {}
			local s = string.format("ffffff%d",i)
			s = s .. tostring(i)
			local f = function(str)
				str = str .. str
			end
			local s1 = tostring(math.random(i))
			local m = {s,s1,f}
			f(s)
			table.insert(all,s)
			out[k] = table.concat(all,"#,@")
			A[k][i] = m
		end
		-- collectgarbage("collect")
	end
end
local function test_vector()
	local dt = 0.017
	for k = 1,30000 do
		for q = 1,10 do
			local p = {x = math.random(),y = math.random()}
			local v = {x = math.random(),y = math.random()}
			p.x = v.x * dt
			p.y = v.y * dt
		end
		-- if k % 1000 == 0 then
		-- 	collectgarbage("collect")
		-- end
	end
end

local function test_small_strings()
	for k = 1,300000 do
		for q = 1,10 do
			local x = math.random()
			local y = math.random()
			local s = "x:" .. tostring(x) .. "," .. "y:" .. tostring(y)
		end
		-- if k % 1000 == 0 then
		-- 	collectgarbage("collect")
		-- end
	end
end

local tests = {
	-- {"test_mix",test_mix},
	{"test_vector",test_vector},
	{"test_small_strings",test_small_strings},
}
for _,test in ipairs(tests) do
	local name = test[1]
	local func = test[2]
	-- collectgarbage("collect")
	func()--预热缓存,warm up chunks
	collectgarbage("collect")
	local t1 = os.clock()
	func()
	local t2 = os.clock()
	local t = t2 - t1
	print(string.format("%s use time:%.6f",name,t))
end
dumpStat()