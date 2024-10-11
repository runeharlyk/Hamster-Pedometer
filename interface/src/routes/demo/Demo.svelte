<script lang="ts">
	import { onMount, onDestroy } from 'svelte';
	import SettingsCard from '$lib/components/SettingsCard.svelte';
	import { socket } from '$lib/stores/socket';
	import Spinner from '$lib/components/Spinner.svelte';
	import MdiDelete from '~icons/mdi/delete';

	let isLoading = true;

	let allTimeTopPace: number;
	let allTimeAveragePace: number;
	let totalDistance: number;
	let totalTimeInWheel: number;

	let statifiedSessions;

	let currentSession: {
		distance: number;
		topPace: number;
		end: number;
		start: number;
		timeRunning: number;
		averagePace: number;
		sessionMin: number;
	};

	let speedMetersPerSecond = 0;

	let isRunning = false;
	let stopTimer: string | number | NodeJS.Timeout | undefined;

	const diameterOfHamsterWheel = 0.19;
	const numberOfMagnets = 1;
	const circumference = (Math.PI * diameterOfHamsterWheel) / numberOfMagnets;

	type Session = {
		start: number;
		end: number;
		steps: number;
		times: number[];
	};

	type Sessions = Session[];

	let sessions: Sessions;

	type PacePackage = { time_elapsed: number };

	onMount(async () => {
		await getSessions();

		socket.on('step', (newData: PacePackage) => {
			if (!isRunning) {
				getSessions();
			}
			isRunning = true;

			speedMetersPerSecond = circumference / newData.time_elapsed;
			sessions[sessions.length - 1].times.push(newData.time_elapsed);
			sessions[sessions.length - 1].steps += 1;
			calculateStats();
			clearTimeout(stopTimer);
			stopTimer = setTimeout(() => {
				isRunning = false;
				speedMetersPerSecond = 0;
			}, 2000);
		});

		calculateStats();
		isLoading = false;
	});

	const getSessions = async () => {
		const response = await fetch('rest/steps');
		const json = await response.json();
		sessions = json.sessions;
	};

	const calculateStats = () => {
		statifiedSessions = sessions.map((session) => statify(session));

		currentSession = statifiedSessions[statifiedSessions.length - 1];

		totalDistance = sum(statifiedSessions.map((sess) => sess.distance));

		allTimeTopPace = Math.max(...statifiedSessions.map((sess) => sess.topPace), 0);

		allTimeAveragePace =
			sum(statifiedSessions.map((sess) => sess.averagePace)) / sessions.length || 0;

		totalTimeInWheel = sum(
			sessions.map((sess) => {
				if (sess.end < 1700000000 || sess.start < 1700000000) return 0;
				return sess.end - sess.start;
			})
		);
	};

	const statify = (session: Session) => {
		const distance = session.steps * circumference;
		const speeds = session.times.map((time) => (time === 0 ? 0 : circumference / time));
		const timeRunning = sum(speeds) || 0;
		const topPace = Math.max(...speeds);
		const averagePace = speeds.length ? timeRunning / speeds.length : 0;
		const sessionMin = session.end - session.start;
		return {
			end: session.end,
			start: session.start,
			distance,
			timeRunning,
			topPace,
			averagePace,
			sessionMin
		};
	};

	const sum = (arr: number[]) => arr.reduce((total, current) => (total += current), 0);

	onDestroy(() => socket.off('step'));

	let rotationAngle = 0;
	let lastTime = 0;
	const speedControl = 0.5;

	const rotateDiv = (time: number) => {
		if (lastTime) {
			const deltaTime = time - lastTime;
			rotationAngle -= speedMetersPerSecond * deltaTime * speedControl;
		}
		lastTime = time;
		requestAnimationFrame(rotateDiv);
	};

	const reset = () => socket.sendEvent('reset_pedometer', '');

	function formatTime(seconds: number): string {
		const h = Math.floor(seconds / 3600);
		const m = Math.floor((seconds % 3600) / 60);

		if (h > 0) {
			return m > 0
				? `${h} hour${h > 1 ? 's' : ''} ${m} min${m > 1 ? 's' : ''}`
				: `${h} hour${h > 1 ? 's' : ''}`;
		}
		return m > 0 ? `${m} min${m > 1 ? 's' : ''}` : `< 1 min`;
	}

	function utcToHHMM(utc: number): string {
		const date = new Date(utc * 1000); // Convert to milliseconds if it's in seconds
		const hours = date.getUTCHours();
		const minutes = date.getUTCMinutes();
		return `${String(hours).padStart(2, '0')}:${String(minutes).padStart(2, '0')}`;
	}
</script>

<SettingsCard collapsible={false}>
	<button class="btn absolute top-2 right-2" on:click={reset}>
		<MdiDelete class="w-8 h-8" />
	</button>

	<div class="container" style="--speed: {1 / speedMetersPerSecond}s">
		<div
			class="wheel-and-hamster"
			role="img"
			aria-label="Orange and tan hamster running in a metal wheel"
		>
			<div class="wheel"></div>
			<div class="hamster">
				<div class="hamster__body">
					<div class="hamster__head">
						<div class="hamster__ear"></div>
						<div class="hamster__eye"></div>
						<div class="hamster__nose"></div>
					</div>
					<div class="hamster__limb hamster__limb--fr"></div>
					<div class="hamster__limb hamster__limb--fl"></div>
					<div class="hamster__limb hamster__limb--br"></div>
					<div class="hamster__limb hamster__limb--bl"></div>
					<div class="hamster__tail"></div>
				</div>
			</div>
			<div class="spoke" style="transform: rotate({rotationAngle}deg);"></div>
		</div>
	</div>

	{#if isLoading}
		<Spinner />
	{:else}
		{#if currentSession}
			<h2 class="text-2xl p-4 text-center">{isRunning ? 'Current' : 'Last'} session</h2>
			<div class="stats stats-vertical lg:stats-horizontal shadow">
				<div class="stat">
					<div class="stat-title">Distance</div>
					<div class="stat-value">
						{currentSession.distance.toFixed(1)} m
					</div>
					<div class="stat-desc">
						{utcToHHMM(currentSession.start)} -
						{currentSession.end ? utcToHHMM(currentSession.end) : 'Now'}
						({formatTime(currentSession.sessionMin)})
					</div>
				</div>

				<div class="stat">
					<div class="stat-title">Top pace</div>
					<div class="stat-value">
						{currentSession.topPace.toFixed(1)} m/s
					</div>
					<div class="stat-desc">↗︎</div>
				</div>

				<div class="stat">
					<div class="stat-title">Pace</div>
					<div class="stat-value">
						{speedMetersPerSecond.toFixed(1)} m/s
					</div>
					<!-- <div class="stat-desc">↘︎ 90 (14%)</div> -->
				</div>
			</div>
		{/if}

		<h2 class="text-2xl p-4 text-center">All time stats</h2>
		<div class="stats stats-vertical lg:stats-horizontal shadow">
			<div class="stat">
				<div class="stat-title">Total Distance</div>
				<div class="stat-value">{totalDistance.toFixed(1)} m</div>
				<div class="stat-desc">Oct 11th - Oct 11th ({formatTime(totalTimeInWheel)})</div>
			</div>
			<div class="stat">
				<div class="stat-title">Top Speed</div>
				<div class="stat-value">{allTimeTopPace.toFixed(1)} m/s</div>
				<!-- <div class="stat-desc">↘︎ 90 (14%)</div> -->
			</div>
			<div class="stat">
				<div class="stat-title">Average pace</div>
				<div class="stat-value">{allTimeAveragePace.toFixed(1)} m/s</div>
				<div class="stat-desc">↗︎</div>
			</div>
		</div>
	{/if}
</SettingsCard>

<style>
	.container {
		background: var(--bg);
		color: var(--fg);
		font: 1em/1.5 sans-serif;
		display: grid;
		place-items: center;
	}
	.wheel-and-hamster {
		/* --speed: 1s; */
		position: relative;
		width: 12em;
		height: 12em;
	}
	.wheel,
	.hamster,
	.hamster div,
	.spoke {
		position: absolute;
	}
	.wheel,
	.spoke {
		border-radius: 50%;
		top: 0;
		left: 0;
		width: 100%;
		height: 100%;
	}
	.wheel {
		background: radial-gradient(
			100% 100% at center,
			hsla(0, 0%, 60%, 0) 47.8%,
			hsl(0, 0%, 60%) 48%
		);
		z-index: 2;
	}
	.hamster {
		animation: hamster var(--speed) ease-in-out infinite;
		top: 50%;
		left: calc(50% - 3.5em);
		width: 7em;
		height: 3.75em;
		transform: rotate(4deg) translate(-0.8em, 1.85em);
		transform-origin: 50% 0;
		z-index: 1;
	}
	.hamster__head {
		animation: hamsterHead var(--speed) ease-in-out infinite;
		background: hsl(30, 90%, 55%);
		border-radius: 70% 30% 0 100% / 40% 25% 25% 60%;
		box-shadow:
			0 -0.25em 0 hsl(30, 90%, 80%) inset,
			0.75em -1.55em 0 hsl(30, 90%, 90%) inset;
		top: 0;
		left: -2em;
		width: 2.75em;
		height: 2.5em;
		transform-origin: 100% 50%;
	}
	.hamster__ear {
		animation: hamsterEar var(--speed) ease-in-out infinite;
		background: hsl(0, 90%, 85%);
		border-radius: 50%;
		box-shadow: -0.25em 0 hsl(30, 90%, 55%) inset;
		top: -0.25em;
		right: -0.25em;
		width: 0.75em;
		height: 0.75em;
		transform-origin: 50% 75%;
	}
	.hamster__eye {
		animation: hamsterEye var(--speed) linear infinite;
		background-color: hsl(0, 0%, 0%);
		border-radius: 50%;
		top: 0.375em;
		left: 1.25em;
		width: 0.5em;
		height: 0.5em;
	}
	.hamster__nose {
		background: hsl(0, 90%, 75%);
		border-radius: 35% 65% 85% 15% / 70% 50% 50% 30%;
		top: 0.75em;
		left: 0;
		width: 0.2em;
		height: 0.25em;
	}
	.hamster__body {
		animation: hamsterBody var(--speed) ease-in-out infinite;
		background: hsl(30, 90%, 90%);
		border-radius: 50% 30% 50% 30% / 15% 60% 40% 40%;
		box-shadow:
			0.1em 0.75em 0 hsl(30, 90%, 55%) inset,
			0.15em -0.5em 0 hsl(30, 90%, 80%) inset;
		top: 0.25em;
		left: 2em;
		width: 4.5em;
		height: 3em;
		transform-origin: 17% 50%;
		transform-style: preserve-3d;
	}
	.hamster__limb--fr,
	.hamster__limb--fl {
		clip-path: polygon(0 0, 100% 0, 70% 80%, 60% 100%, 0% 100%, 40% 80%);
		top: 2em;
		left: 0.5em;
		width: 1em;
		height: 1.5em;
		transform-origin: 50% 0;
	}
	.hamster__limb--fr {
		animation: hamsterFRLimb var(--speed) linear infinite;
		background: linear-gradient(hsl(30, 90%, 80%) 80%, hsl(0, 90%, 75%) 80%);
		transform: rotate(15deg) translateZ(-1px);
	}
	.hamster__limb--fl {
		animation: hamsterFLLimb var(--speed) linear infinite;
		background: linear-gradient(hsl(30, 90%, 90%) 80%, hsl(0, 90%, 85%) 80%);
		transform: rotate(15deg);
	}
	.hamster__limb--br,
	.hamster__limb--bl {
		border-radius: 0.75em 0.75em 0 0;
		clip-path: polygon(0 0, 100% 0, 100% 30%, 70% 90%, 70% 100%, 30% 100%, 40% 90%, 0% 30%);
		top: 1em;
		left: 2.8em;
		width: 1.5em;
		height: 2.5em;
		transform-origin: 50% 30%;
	}
	.hamster__limb--br {
		animation: hamsterBRLimb var(--speed) linear infinite;
		background: linear-gradient(hsl(30, 90%, 80%) 90%, hsl(0, 90%, 75%) 90%);
		transform: rotate(-25deg) translateZ(-1px);
	}
	.hamster__limb--bl {
		animation: hamsterBLLimb var(--speed) linear infinite;
		background: linear-gradient(hsl(30, 90%, 90%) 90%, hsl(0, 90%, 85%) 90%);
		transform: rotate(-25deg);
	}
	.hamster__tail {
		animation: hamsterTail var(--speed) linear infinite;
		background: hsl(0, 90%, 85%);
		border-radius: 0.25em 50% 50% 0.25em;
		box-shadow: 0 -0.2em 0 hsl(0, 90%, 75%) inset;
		top: 1.5em;
		right: -0.5em;
		width: 1em;
		height: 0.5em;
		transform: rotate(30deg) translateZ(-1px);
		transform-origin: 0.25em 0.25em;
	}
	.spoke {
		/* animation: spoke var(--speed) linear infinite; */
		background:
			radial-gradient(100% 100% at center, hsl(0, 0%, 60%) 4.8%, hsla(0, 0%, 60%, 0) 5%),
			linear-gradient(hsla(0, 0%, 55%, 0) 46.9%, hsl(0, 0%, 65%) 47% 52.9%, hsla(0, 0%, 65%, 0) 53%)
				50% 50% / 99% 99% no-repeat;
	}

	/* Animations */
	@keyframes hamster {
		from,
		to {
			transform: rotate(4deg) translate(-0.8em, 1.85em);
		}
		50% {
			transform: rotate(0) translate(-0.8em, 1.85em);
		}
	}
	@keyframes hamsterHead {
		from,
		25%,
		50%,
		75%,
		to {
			transform: rotate(0);
		}
		12.5%,
		37.5%,
		62.5%,
		87.5% {
			transform: rotate(8deg);
		}
	}
	@keyframes hamsterEye {
		from,
		90%,
		to {
			transform: scaleY(1);
		}
		95% {
			transform: scaleY(0);
		}
	}
	@keyframes hamsterEar {
		from,
		25%,
		50%,
		75%,
		to {
			transform: rotate(0);
		}
		12.5%,
		37.5%,
		62.5%,
		87.5% {
			transform: rotate(12deg);
		}
	}
	@keyframes hamsterBody {
		from,
		25%,
		50%,
		75%,
		to {
			transform: rotate(0);
		}
		12.5%,
		37.5%,
		62.5%,
		87.5% {
			transform: rotate(-2deg);
		}
	}
	@keyframes hamsterFRLimb {
		from,
		25%,
		50%,
		75%,
		to {
			transform: rotate(50deg) translateZ(-1px);
		}
		12.5%,
		37.5%,
		62.5%,
		87.5% {
			transform: rotate(-30deg) translateZ(-1px);
		}
	}
	@keyframes hamsterFLLimb {
		from,
		25%,
		50%,
		75%,
		to {
			transform: rotate(-30deg);
		}
		12.5%,
		37.5%,
		62.5%,
		87.5% {
			transform: rotate(50deg);
		}
	}
	@keyframes hamsterBRLimb {
		from,
		25%,
		50%,
		75%,
		to {
			transform: rotate(-60deg) translateZ(-1px);
		}
		12.5%,
		37.5%,
		62.5%,
		87.5% {
			transform: rotate(20deg) translateZ(-1px);
		}
	}
	@keyframes hamsterBLLimb {
		from,
		25%,
		50%,
		75%,
		to {
			transform: rotate(20deg);
		}
		12.5%,
		37.5%,
		62.5%,
		87.5% {
			transform: rotate(-60deg);
		}
	}
	@keyframes hamsterTail {
		from,
		25%,
		50%,
		75%,
		to {
			transform: rotate(30deg) translateZ(-1px);
		}
		12.5%,
		37.5%,
		62.5%,
		87.5% {
			transform: rotate(10deg) translateZ(-1px);
		}
	}
	@keyframes spoke {
		from {
			transform: rotate(0);
		}
		to {
			transform: rotate(-1turn);
		}
	}
</style>
